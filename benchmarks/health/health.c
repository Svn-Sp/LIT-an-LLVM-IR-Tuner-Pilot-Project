#include <libgen.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "health.h"

int max_level;
long max_time;
long long seed;

struct Village *alloc_tree(int level, int label, struct Village *back) {
    if (level == 0) {
        return NULL;
    }

    struct Village *new_v = (struct Village *)malloc(sizeof(struct Village));
    struct Village *fval[4];
    for (int i = 3; i >= 0; i--) {
        fval[i] = alloc_tree(level - 1, label * 4 + i + 1, new_v);
    }

    new_v->back = back;
    new_v->label = label;
    new_v->seed = label * (IQ + seed);
    new_v->hosp.personnel = (int)pow(2, level - 1);
    new_v->hosp.free_personnel = new_v->hosp.personnel;
    new_v->hosp.num_waiting_patients = 0;
    new_v->hosp.assess.forward = NULL;
    new_v->hosp.assess.back = NULL;
    new_v->hosp.assess.patient = NULL;
    new_v->hosp.waiting.forward = NULL;
    new_v->hosp.waiting.back = NULL;
    new_v->hosp.waiting.patient = NULL;
    new_v->hosp.inside.forward = NULL;
    new_v->hosp.inside.back = NULL;
    new_v->hosp.inside.patient = NULL;
    new_v->hosp.up.forward = NULL;
    new_v->hosp.up.back = NULL;
    new_v->hosp.up.patient = NULL;
    new_v->returned.back = NULL;
    new_v->returned.forward = NULL;
    new_v->returned.patient = NULL;

    for (int i = 0; i < 4; i++) {
        new_v->forward[i] = fval[i];
    }
    return new_v;
}

struct Results get_results(struct Village *village) {
    struct Results r = {0.0f, 0.0f, 0.0f};
    if (village == NULL) {
        return r;
    }

    struct Results fval[4];
    for (int i = 3; i > 0; i--) {
        fval[i] = get_results(village->forward[i]);
    }
    fval[0] = get_results(village->forward[0]);

    for (int i = 3; i >= 0; i--) {
        r.total_hosps += fval[i].total_hosps;
        r.total_patients += fval[i].total_patients;
        r.total_time += fval[i].total_time;
    }

    struct List *list = village->returned.forward;
    while (list != NULL) {
        struct Patient *p = list->patient;
        r.total_hosps += (float)(p->hosps_visited);
        r.total_time += (float)(p->time);
        r.total_patients += 1.0f;
        list = list->forward;
    }

    return r;
}

OPT_ATTR void check_patients_inside(struct Village *village, struct List *list) {
    while (list != NULL) {
        struct Patient *p = list->patient;
        p->time_left -= 1;
        if (p->time_left == 0) {
            village->hosp.free_personnel += 1;
            removeList(&village->hosp.inside, p);
            addList(&village->returned, p);
        }
        list = list->forward;
    }
}

OPT_ATTR struct List *check_patients_assess(struct Village *village, struct List *list) {
    struct List *up = NULL;
    while (list != NULL) {
        struct Patient *p = list->patient;
        p->time_left -= 1;
        if (p->time_left == 0) {
            float r = my_rand(village->seed);
            village->seed = (long long)(r * IM);
            if (r > 0.1f || village->label == 0) {
                removeList(&village->hosp.assess, p);
                addList(&village->hosp.inside, p);
                p->time_left = 10;
                p->time += 10;
            } else {
                village->hosp.free_personnel += 1;
                removeList(&village->hosp.assess, p);
                up = &village->hosp.up;
                addList(up, p);
            }
        }
        list = list->forward;
    }
    return up;
}

OPT_ATTR void check_patients_waiting(struct Village *village, struct List *list) {
    while (list != NULL) {
        struct Patient *p = list->patient;
        if (village->hosp.free_personnel > 0) {
            village->hosp.free_personnel -= 1;
            p->time_left = 3;
            p->time += 3;
            removeList(&village->hosp.waiting, p);
            addList(&village->hosp.assess, p);
        } else {
            p->time += 1;
        }
        list = list->forward;
    }
}

OPT_ATTR void put_in_hosp(struct Hosp *hosp, struct Patient *patient) {
    patient->hosps_visited += 1;
    if (hosp->free_personnel > 0) {
        hosp->free_personnel -= 1;
        addList(&hosp->assess, patient);
        patient->time_left = 3;
        patient->time += 3;
    } else {
        addList(&hosp->waiting, patient);
    }
}

OPT_ATTR struct Patient *generate_patient(struct Village *village) {
    float r = my_rand(village->seed);
    village->seed = (long long)(r * IM);
    if (r > 0.666f) {
        struct Patient *patient = (struct Patient *)malloc(sizeof(struct Patient));
        patient->hosps_visited = 0;
        patient->time = 0;
        patient->time_left = 0;
        patient->home_village = village;
        return patient;
    }
    return NULL;
}

OPT_ATTR struct List *sim(struct Village *village) {
    if (village == NULL) {
        return NULL;
    }

    struct List *val[4];
    for (int i = 3; i > 0; i--) {
        val[i] = sim(village->forward[i]);
    }
    val[0] = sim(village->forward[0]);

    struct Hosp *h = &village->hosp;
    for (int i = 3; i >= 0; i--) {
        struct List *val_i = val[i];
        struct List *l = val_i;
        if (l != NULL) {
            l = l->forward;
            while (l != NULL) {
                put_in_hosp(h, l->patient);
                removeList(val_i, l->patient);
                l = l->forward;
            }
        }
    }

    check_patients_inside(village, village->hosp.inside.forward);
    struct List *up = check_patients_assess(village, village->hosp.assess.forward);
    check_patients_waiting(village, village->hosp.waiting.forward);

    struct Patient *patient = generate_patient(village);
    if (patient != NULL) {
        put_in_hosp(&village->hosp, patient);
    }
    return up;
}

int main(int argc, char *argv[]) {
    dealwithargs(argc, argv);
    struct Village *top = alloc_tree(max_level, 0, NULL);

    for (int i = 0; i < max_time; i++) {
        sim(top);
    }

    struct Results results = get_results(top);
    float total_patients = results.total_patients;
    float total_time = results.total_time;
    float total_hosps = results.total_hosps;

    // Deterministic scalar suitable for framework's scalar comparator.
    float score = total_patients + total_time + total_hosps;

    char filepath[512];
    strcpy(filepath, __FILE__);
    char *dir = dirname(filepath);
    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/output.txt", dir);
    FILE *file = fopen(output_path, "w");
    if (!file) {
        return 1;
    }
    fprintf(file, "%.8f\n", score);
    fclose(file);

    printf("patients=%f avg_stay=%0.2f avg_hosp=%f score=%.8f\n",
           total_patients,
           total_time / total_patients,
           total_hosps / total_patients,
           score);
    return 0;
}
