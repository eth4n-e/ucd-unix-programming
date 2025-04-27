#!/bin/bash
source ./assignment4-data/stage1.sh.inc
source ./assignment4-data/stage2.sh.inc
source ./assignment4-data/stage3.sh.inc
source ./assignment4-data/stage4.sh.inc
source ./assignment4-data/grades.sh.inc
source ./assignment4-data/mincredits.sh.inc

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
