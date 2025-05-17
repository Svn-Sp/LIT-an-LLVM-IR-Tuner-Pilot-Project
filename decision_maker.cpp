#pragma once
#include <random>
#include <iostream>

class DecisionMaker {
    public:
        int* decisions;
        int num_decisions;
        bool new_decisions = true;
        int current_decision = 0;
        std::random_device rd;
        std::mt19937 gen;

        // Constructor with size parameter
        DecisionMaker(int num_decisions){
            this->num_decisions = num_decisions;
            this->decisions = new int[num_decisions]();
            this->gen = std::mt19937(this->rd());
        }
        
        DecisionMaker(int num_decisions, int decisions[]){
            this->num_decisions = num_decisions;
            this->decisions = new int[num_decisions];
            for(int i = 0; i < num_decisions; i++) {
                this->decisions[i] = decisions[i];
            }
            this->new_decisions = false;
        }

        // Copy assignment operator
        DecisionMaker& operator=(const DecisionMaker& other) {
            if (this != &other) {  // Check for self-assignment
                // Free existing resources
                delete[] this->decisions;
                
                // Copy data from other object
                this->num_decisions = other.num_decisions;
                this->new_decisions = other.new_decisions;
                this->current_decision = other.current_decision;
                this->gen = other.gen;
                
                // Allocate and copy the array
                this->decisions = new int[num_decisions];
                for(int i = 0; i < num_decisions; i++) {
                    this->decisions[i] = other.decisions[i];
                }
            }
            return *this;
        }
        
        // Destructor
        ~DecisionMaker() {
            delete[] decisions;
        }
        
        int make_decision(int min, int max){
            if (current_decision >= num_decisions) {
                // Reset or handle out of bounds
                std::cerr << "Warning: decision index out of bounds, resetting to 0" << std::endl;
                current_decision = 0;
            }
            if (new_decisions){
                std::uniform_int_distribution<> dis(min, max);
                int decision = dis(this->gen);
                this->decisions[this->current_decision] = decision;
                this->current_decision++;
                return decision;
            }

            int decision = this->decisions[this->current_decision];
            this->current_decision++;
            return decision;
        }

        void print_decisions(){
            std::cout << "DECISIONS:[";
            for(int i = 0; i < num_decisions; i++) {
                std::cout << decisions[i];
                if (i < num_decisions - 1) {
                    std::cout << ",";
                }
            }
            std::cout << "]" << std::endl;
        }
};