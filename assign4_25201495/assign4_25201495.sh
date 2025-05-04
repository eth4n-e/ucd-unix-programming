#!/opt/homebrew/bin/bash
# Name: Ethan Epperson
# Student Number: 25201495
# Email: ethan.epperson@ucdconnect.ie
# Usage:
# NOTE: bash script requires bash version>4.3 because use of namerefs
# NOTE: my homebrew installed bash version is located as /opt/homebrew/bin/bash 
# NOTE: and the homebrew installed version is greater than 4.3
# Run with: $ ./assign4_25201495.sh <I|NI>
# Script takes in an argument I or NI which specifies whether Industry Internship is taken as an elective in Stage 3
# for each stage the script:
# - populates a global list of courses
# - checks if more courses need to be taken
# - generates electives to take if necessary
# - randomly generates letter grades and grade points for courses
# - displays all courses taken within the stage
# - sums the grade point total across all stages
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
COURSES=()
CREDIT_HOURS=0
GRADE_POINT_TOTAL=0
INTERNSHIP=0
declare -a internship=("${stage3elective[0]}" "${stage3elective[1]}" "${stage3elective[2]}")

### FUNCTIONS USED THROUGHOUT SCRIPT
# input: element 
# output: true / false if array contains element
function courses_contains_element {
    local element="$1"
    local courses_len="${#COURSES[@]}"
    for (( i=0; i<$courses_len; i+=3 )) do
        # module codes appear every 3 indices
        # check if they match
        if [[ "$element" == "${COURSES[i]}" ]]; then
            return 0  # true
        fi
    done
    return 1  # false
}

# input: array of courses
function add_credit_hours {
    # create a local variable for array passed to function 
    local courses=("$@")

    # note: a courses credit hours falls at i % 3 == 2
    # index in array where i is index
    for (( i=2; i<"${#courses[@]}"; i+=3 )) do
        # add credit hours
        CREDIT_HOURS=$((CREDIT_HOURS + courses[i]))
    done

    return "$CREDIT_HOURS"
}

# input: letter grade variable to populate 
# output: letter grade
function generate_letter_grade {
    # grades is organized such that a grade, grade point combo represents 2 elements
    # there are 5 grade, grade point pairs
    # grade_row represents a pair number (e.g. pair 4 = D 2)
    # multiply by ENTRIES_PER_GRADE to select the grade letter
    local num_grades=$(("${#grades[@]}" / "$ENTRIES_PER_GRADE"))
    local grade_row=$(($RANDOM % "$num_grades"))
    local letter_grade_idx=$(("$grade_row" * "$ENTRIES_PER_GRADE"))
    # select the letter grade
    local letter_grade="${grades[$letter_grade_idx]}"
    echo "$letter_grade"
}

# output: grade point
function generate_grade_point {
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
# output: return starting index of an elective record
function select_elective {
    local electives=("$@")
    # 3 entries per course, divide to get number of courses
    local num_electives=$(("${#electives[@]}" / "$ENTRIES_PER_COURSE"))
    # generate random index to retrieve elective
    local elec_idx=$((RANDOM % "$num_electives"))

    return $(("$elec_idx" * $ENTRIES_PER_COURSE))
}

# input: name of an elective tuple, name of courses list
function generate_elective {
    # create namerefs to arguments, changes made within function reflect in that variable outside
    local electives=("$@")
    local elective=()
    local elec_idx
    local module
    local title
    local cred
    # pass the name of the array to populate 
    # select_elective creates a nameref so that changes made in function reflect in variable
    select_elective "${electives[@]}"
    elec_idx="$?"
    # populate the elective
    module="${electives[elec_idx]}"
    title="${electives[elec_idx+1]}"
    cred="${electives[elec_idx+2]}"
    elective=("$module" "$title" "$cred")
    # generate unique electives
    while courses_contains_element "$module"; do
        # reset elective to re-populate it
        select_elective "${electives[@]}"
        elec_idx="$?"

        module="${electives[elec_idx]}"
        title="${electives[elec_idx+1]}"
        cred="${electives[elec_idx+2]}"
        elective=("$module" "$title" "$cred")
    done
    # add the elective to our course list
    COURSES+=("${elective[@]}")
    CREDIT_HOURS=$((CREDIT_HOURS + "${elective[2]}"))
}

# input: stage number
# output: list of courses to take
function populate_stage {
    local stage="$1"
    # dynamically reference array name based on stage number
    local stage_core_name="stage${stage}core"
    local stage_elec_name="stage${stage}elective"
    local stage_min_cred_name="STAGE${stage}_MIN_CREDITS"
    # uses namerefs - creates reference to variable whose name stored in stage_..._name
    declare -n cur_stage_core="$stage_core_name"
    declare -n cur_stage_elec="$stage_elec_name"
    declare -n cur_stage_min_cred="$stage_min_cred_name"

    # populate courses with core requirements
    COURSES+=("${cur_stage_core[@]}")
    # calculate accumulated credit hours from core courses
    add_credit_hours "${cur_stage_core[@]}" 
    CREDIT_HOURS="$?"

    # internship selected
    if [ "$INTERNSHIP" -eq 1 ] && [ "$stage" == "3" ]; then
        COURSES+=("${internship[@]}")
        CREDIT_HOURS=$((CREDIT_HOURS + "${internship[2]}"))
    fi

    # student needs more credits, randomly select from electives at that stage
    while [ "$CREDIT_HOURS" -lt "$cur_stage_min_cred" ]; do
        # generate a unique elective from inputted list to add to course list
        generate_elective "${cur_stage_elec[@]}"
    done
}

# input: stage number, list of courses being taken
function print_stage {
    local stage="$1"
    local num_courses=$(("${#COURSES[@]}"/"$ENTRIES_PER_COURSE"))

    echo Stage $stage
    echo Module, Title, Cred, Grade, G.P
    for (( course_num=0; course_num < "$num_courses"; course_num++ ));
    do
        # each record has 5 entries
        # so record_idx is at record_num * 5
        # e.g. record 1 starts at index 5
        local start_idx=$(("$course_num" * "$ENTRIES_PER_COURSE"))
        local module_code="${COURSES[start_idx]}"
        local course_name="${COURSES[start_idx+1]}"
        local credit_hours="${COURSES[start_idx+2]}"
        # generate grade and grade point for each course
        # use command substitution to capture echoed value
        local letter_grade=$(generate_letter_grade)
        generate_grade_point
        local grade_point="$?"
        # update grade point total
        GRADE_POINT_TOTAL=$(("$GRADE_POINT_TOTAL" + "$grade_point"))
        echo "$module_code", "$course_name", "$credit_hours", "$letter_grade", "$grade_point"
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
for stage in ${STAGES[@]}; do
    populate_stage "$stage" 
    print_stage "$stage" 
    echo -e "\n"
    # reset courses and credit hours for next stage
    COURSES=()
    CREDIT_HOURS=0
done

echo Total grade point score = "$GRADE_POINT_TOTAL".
