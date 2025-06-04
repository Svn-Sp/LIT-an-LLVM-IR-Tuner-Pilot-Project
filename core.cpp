#ifndef CORE_CPP
#define CORE_CPP

#include <vector>
#include <tuple>
#include <sstream>
#include <cmath>
#include <sqlite3.h>
#include "mutation.cpp"
#include "constants.h"
class Run {
    public:
    std::vector<std::tuple<MutationType, std::vector<int>>> mutations;
    double avgDuration;
    double stddevDuration;
    double result;
    std::string asString(){
        std::stringstream ss;
        ss << "Run: " << avgDuration << " " << stddevDuration << "\n";
        for (std::tuple<MutationType, std::vector<int>> mutation : mutations) {
            ss << std::get<0>(mutation) << " ";
            for(int i : std::get<1>(mutation)){
                ss << i << " ";
            }
            ss << "\n";
        }
        return ss.str();
    }
    
    bool saveToDb() {
        sqlite3* db;
        char* errMsg = nullptr;
        int rc;
        
        // Open database connection
        rc = sqlite3_open(DB_PATH, &db);
        if (rc) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return false;
        }
        
        // Begin transaction
        rc = sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        // Insert into run table (ignoring code_id for now)
        std::string runSql = "INSERT INTO run (code_id, result, avg_rt, std_dev_rt) VALUES (NULL, ?, ?, ?)";
        sqlite3_stmt* runStmt;
        rc = sqlite3_prepare_v2(db, runSql.c_str(), -1, &runStmt, nullptr);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error in prepare: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return false;
        }
        
        if (std::isfinite(result)) {
            sqlite3_bind_double(runStmt, 1, result);
        } else {
            sqlite3_bind_null(runStmt, 1);
        }
        sqlite3_bind_double(runStmt, 2, avgDuration);
        sqlite3_bind_double(runStmt, 3, stddevDuration);
        
        rc = sqlite3_step(runStmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "SQL error in step: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(runStmt);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_finalize(runStmt);
        
        // Get the last inserted run_id
        sqlite3_int64 runId = sqlite3_last_insert_rowid(db);
        
        // Insert mutations
        std::string mutSql = "INSERT INTO mutation (step, run_id, mutation_type, decisions) VALUES (?, ?, ?, ?)";
        sqlite3_stmt* mutStmt;
        rc = sqlite3_prepare_v2(db, mutSql.c_str(), -1, &mutStmt, nullptr);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error in prepare mutations: %s\n", sqlite3_errmsg(db));
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            sqlite3_close(db);
            return false;
        }
        
        for (size_t i = 0; i < mutations.size(); i++) {
            auto& mutation = mutations[i];
            MutationType type = std::get<0>(mutation);
            std::vector<int>& decisions = std::get<1>(mutation);
            
            // Convert mutation type to string
            std::string typeStr;
            switch(type) {
                case ADD_RANDOM_ARITHMETIC: typeStr = "ADD_RANDOM_ARITHMETIC"; break;
                case REPLACE_ARITHMETIC: typeStr = "REPLACE_ARITHMETIC"; break;
                case MOVE_BLOCKWISE: typeStr = "MOVE_BLOCKWISE"; break;
                case ADD_NEW_COND: typeStr = "ADD_NEW_COND"; break;
                default: typeStr = "UNKNOWN"; break;
            }
            
            // Convert decisions vector to JSON-like string
            std::stringstream decisionsStr;
            decisionsStr << "[";
            for (size_t j = 0; j < decisions.size(); j++) {
                decisionsStr << decisions[j];
                if (j < decisions.size() - 1) {
                    decisionsStr << ",";
                }
            }
            decisionsStr << "]";
            
            // Bind parameters
            sqlite3_bind_int(mutStmt, 1, static_cast<int>(i));  // step
            sqlite3_bind_int64(mutStmt, 2, runId);  // run_id
            sqlite3_bind_text(mutStmt, 3, typeStr.c_str(), -1, SQLITE_TRANSIENT);  // mutation_type
            sqlite3_bind_text(mutStmt, 4, decisionsStr.str().c_str(), -1, SQLITE_TRANSIENT);  // decisions
            
            rc = sqlite3_step(mutStmt);
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "SQL error in step mutations: %s\n", sqlite3_errmsg(db));
                sqlite3_finalize(mutStmt);
                sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
                sqlite3_close(db);
                return false;
            }
            
            sqlite3_reset(mutStmt);
            sqlite3_clear_bindings(mutStmt);
        }
        
        sqlite3_finalize(mutStmt);
        
        // Commit transaction
        rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error in commit: %s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_close(db);
        return true;
    }
};

#endif // CORE_CPP