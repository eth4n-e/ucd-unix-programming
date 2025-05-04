#!/bin/bash
source ./assignment4-data/stage1.sh.inc
source ./assignment4-data/stage2.sh.inc
source ./assignment4-data/stage3.sh.inc
source ./assignment4-data/stage4.sh.inc
source ./assignment4-data/grades.sh.inc
source ./assignment4-data/mincredits.sh.inc

# declaration of useful global variables
ENTRIES_PER_COURSE=3
ENTRIES_PER_GRADE=2
HELP_FLAG="--help"
STAGE1_MIN_CREDITS=${minc[0]}
STAGE2_MIN_CREDITS=${minc[1]}
STAGE3_MIN_CREDITS=${minc[2]}
STAGE4_MIN_CREDITS=${minc[3]}
STAGES=(1 2 3 4)
GRADE_POINT_TOTAL=0
INTERNSHIP=0
declare -a internship=("${stage3elective[0]}" "${stage3elective[1]}" "${stage3elective[2]}")

### FUNCTIONS USED THROUGHOUT SCRIPT
# input: element, array
# output: true / false if array contains element
function contains_element {
    echo in contains element
    local -n element="$1"
    shift
    local -n array="$2"
    for (( i = 0; i < "${#array[@]}"; i+=3 )) do
        # module codes appear every 3 indices
        # check if they match
        if [[ "${element[0]}" == "${array[i]}" ]]; then
            return 0  # true
        fi
    done
    return 1  # false
}

# input: array of courses
function add_credit_hours {
    echo in add credit hours
    # create a local variable for array passed to function 
    local -n courses="$1"
    local credit_hours=0

    # note: a courses credit hours falls at i % 3 == 2
    # index in array where i is index
    for ((i=2; i < "${#courses[@]}"; i+=3 )) do
        # add credit hours
        credit_hours=$((credit_hours + courses[i]))
    done

    return "$credit_hours"
}

# input: letter grade variable to populate 
# output: letter grade
function generate_letter_grade() {
    echo in generate letter grade
    local -n letter_grade="$1"
    # grades is organized such that a grade, grade point combo represents 2 elements
    # there are 5 grade, grade point pairs
    # grade_row represents a pair number (e.g. pair 4 = D 2)
    # multiply by ENTRIES_PER_GRADE to select the grade letter
    num_grades=$(("${#grades[@]}" / "$ENTRIES_PER_GRADE"))
    grade_row=$(($RANDOM % "$num_grades"))
    letter_grade_idx=$(("$grade_row" * "$ENTRIES_PER_GRADE"))
    # select the letter grade
    letter_grade="${grades[$letter_grade_idx]}"
}

# output: grade point
function generate_grade_point() {
    echo in generate grade point
    # grades is organized such that a grade, grade point combo represents 2 elements
    # there are 5 grade, grade point pairs
    # grade_row represents a pair number (e.g. pair 4 = D 2)
    # multiply by ENTRIES_PER_GRADE and add 1 to select grade point
    num_grades=$(("${#grades[@]}" / "$ENTRIES_PER_GRADE"))
    grade_row=$(($RANDOM % "$num_grades"))
    grade_point_idx=$(("$grade_row" * "$ENTRIES_PER_GRADE" + 1))
    return "${grades[$grade_point_idx]}"
}

# input: name of elective array to populate, name of list of electives to select from
# output: none, variables are modified with use of namerefs
function select_elective {
    echo in select_elective
    local -n elective=$1
    local -n electives=$2
    # 3 entries per course, divide to get number of courses
    local num_electives=$(("${#electives[@]}" / "$ENTRIES_PER_COURSE"))
    # generate random index to retrieve elective
    local rand_idx=$((RANDOM % "$num_electives"))

    # populate the elective
    elective+=("${electives[rand_idx]}" "${electives[rand_idx+1]}" "${electives[rand_idx+2]}")
}

# input: name of an elective tuple, name of courses list
function generate_elective() {
    echo in generate_elective
    # create namerefs to arguments, changes made within function reflect in that variable outside
    local -n electives="$1"
    shift
    local -n courses="$2"
    local elective=()
    # pass the name of the array to populate 
    # select_elective creates a nameref so that changes made in function reflect in variable
    select_elective elective "$1" 
    # generate unique electives
    while [ contains_element "$1" "$2" ]; do
        # reset elective to re-populate it
        elective=()
        select_elective elective "$1" 
    done
    # add the elective to our course list
    courses+=("${elective[@]}")
}

# input: stage number
# output: list of courses to take
function populate_stage() {
    echo in populate stage
    local stage="$1"
    local -n courses="$2"
    # dynamically reference array name based on stage number
    local stage_core_name="$stage${stage}core"
    local stage_min_cred_name="$STAGE${stage}_MIN_CREDITS"
    # uses namerefs - creates reference to variable whose name stored in stage_..._name
    declare -n cur_stage_core="$stage_core_name"
    declare -n cur_stage_min_cred="$stage_min_cred_name"

    add_credit_hours "$stage_core_name" 
    credit_hours="$?"

    # internship selected
    if [ "$INTERNSHIP" -eq 1 ] && [ "$stage" -eq 3 ]; then
        courses+=("$internship")
    fi

    # student needs more credits, randomly select from electives at that stage
    while [ "$credit_hours" -lt "$cur_stage_min_cred" ]; do
        local elective=()
        # pass original variable name so that invoked function can access
        generate_elective elective "$stage_core_name" 
        courses+="${elective[@]}"
    done
}

# input: stage number, list of courses being taken
function print_stage() {
    echo in print stage
    local stage="$1"
    shift
    local courses=("$@")
    local counter=0
    local num_courses=$(("${#courses[@]}"/"$ENTRIES_PER_COURSE"))

    echo Stage $stage
    echo -e Module Code\t\tCourse Name\t\tCredit Hours\t\tGrade Point
    for (( course_num=0; course_num < "$num_courses"; course_num++ ));
    do
        # each record has 5 entries
        # so record_idx is at record_num * 5
        # e.g. record 1 starts at index 5
        local start_idx=$(("$course_num" * "$ENTRIES_PER_COURSE"))
        local module_code=${courses[$start_idx]}
        local course_name=${courses[$((start_idx+1))]}
        local credit_hours=${courses[$((start_idx+2))]}
        # generate grade and grade point for each course
        local letter_grade
        generate_letter_grade letter_grade
        generate_grade_point
        local grade_point="$?"
        # update grade point total
        $((GRADE_POINT_TOTAL+=grade_point))
        echo $module_code $course_name $credit_hours $letter_grade $grade_point
    done

    return
}
### END OF FUNCTIONS USED IN SCRIPT

num_args=$#
# reading user input
if [ "$num_args" -lt "1" -o "$1" == "$HELP_FLAG" ]; then
    echo -e "Correct usage: bsctranscript.sh <I|NI>\nI for internship in stage 3\nNI for no internship in stage 3"
    exit 1
fi

if [ "$1" == "I" ]; then
    INTERNSHIP=1
fi

# for each stage, populate and display that stage
for stage in ${stages[@]}; do
    courses=()
    populate_stage "$stage"
    print_stage "$stage" "${courses[@]}"
done

echo Total grade point score = "$GRADE_POINT_TOTAL".

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
