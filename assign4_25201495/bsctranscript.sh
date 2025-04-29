#!/bin/bash
source ./assignment4-data/stage1.sh.inc
source ./assignment4-data/stage2.sh.inc
source ./assignment4-data/stage3.sh.inc
source ./assignment4-data/stage4.sh.inc
source ./assignment4-data/grades.sh.inc
source ./assignment4-data/mincredits.sh.inc

# declaration of functions used throughout script

# input: array of courses
function add_credit_hours {
    # create a local variable for array passed to function 
    local courses=("$@")
    local credit_hours=0

    # "${!courses[@]} gives list of indices
    for i in "${!courses[@]}"; do
        if [ $((i%3)) -eq 2 ]; then
            # note: a courses credit hours falls at i % 3 == 2
            # index in array where i is index
            credit_hours=$((credit_hours + courses[i]))
        fi
    done

    return "$credit_hours"
}

# declaration of variables used throughout script
help_flag="--help"
num_args="$#"
stage1_min_credits=55
stage2_min_credits=50
stage3_min_credits=50
stage4_min_credits=60
stages=(1 2 3 4)

# reading user input
if [ "$num_args" -lt "1" -o "$1" == "$help_flag" ]; then
    echo -e "Correct usage: bsctranscript.sh <I|NI>\nI for internship in stage 3\nNI for no internship in stage 3"
fi

# dynamically reference array name based on stage number
for stage in "${stages[@]}"; do
    local stage_ref="$stage${stage}core"
    local stage_min_cred_ref="$stage${stage}_min_credits"
    # uses namerefs - creates reference to variable whose name stored in stage_ref
    declare -n cur_stage="$stage_ref"
    declare -n cur_stage_min_cred="$stage_min_cred_ref"

    add_credit_hours "${cur_stage}"
    credit_hours="$?"

    # student needs more credits, randomly select from electives at that stage
    if [ "$credit_hours" -lt "$cur_stage_min_cred" ]; then
        
    fi

    # print current stage transcript
done


# Notes:
# Script takes an argument
# If no args, invalid args, or --help provided, must print correct usage
# if grade F is selected, pick another elective 

# Outline:
# read user input to determine Internship / No Internship for stage 3
# output correct usage if script incorrectly invoked
# create a function that adds up the credit hours for modules in a list
# create constants for the minimum number of credit hours for each stage
# have a loop that runs for as many stages that exist
# in the loop:
# maintain a list of courses that the student will take
# add up the credit hours for core courses & add to list of courses
# call a function to check if the minimum number of credit hours is met
# if so, display the modules
# if not, randomly select modules from the electives at 
# that stage and add to the list
# for each course in the list randomly generate a grade / grade point
# maintain a variable that keeps track of the grade point score
# end of loop: display grade point total

# what I need to understand:
# arrays
# looping
# reading input / evaluating arguments
# writing functions
# printing / echoing
# arithmetic
