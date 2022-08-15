#!/bin/bash

# Automatically prepends / updates a basic header comment as needed across both
# the src directory as needed. Only targets *.hpp and *.cpp files currently.

text="/**
 * abbrv Source Code
 * Copyright (C) 2020 Jake Mason
 *
 * @version VER_NUMBER
 * @author Jake Mason
 * @date DATE
 *
 **/
"
prev_version=$1
new_version=$2
header=${text/VER_NUMBER/$2}

exempt_files=(
"../src/classes/External/imgui.cpp"
"../src/classes/External/imgui_draw.cpp"
"../src/classes/External/imgui_impl_sdl.cpp"
"../src/classes/External/imgui_demo.cpp"
"../src/classes/External/imgui_impl_opengl3.cpp"
"../src/classes/External/imgui_sdl.cpp"
"../src/classes/External/imgui_widgets.cpp"
"../src/classes/External/implot.cpp"
"../src/classes/External/imgui_stdlib.cpp"
"../src/classes/External/imgui_tables.cpp"
"../src/classes/External/implot_items.cpp"
"../src/classes/External/implot_demo.cpp"
)

if [ "$#" -lt 2 ]; then
    echo "You must pass two arguments: "
    echo "First, the version you are updating from."
    echo "Second, the version you are updating to."
    echo "No changes made. Exiting."
    exit 1
fi


update_headers(){
    local file=$1
    local date=$(date -r $file "+%m-%d-%Y")
    local current_year=$(date +"%Y")

    for f in ${exempt_files[@]}; do
        if [ "$f" = "$file" ]; then
            return -1
        fi
    done

    # retroactively adding date below the author
    if ! grep -q '@date' $file; then
        sed -i "s|@author Jake Mason|@author Jake Mason\n * @date $date|g" $file
    fi

    if grep -q '@version' $file; then
        sed -i "s|@version $prev_version|@version $new_version|g" $file
        echo "Updated version header in $file"
    fi
    
    if ! grep -q '@version' $file; then
        echo "$header" | cat - "$file" > temp && mv temp $file
        sed -i "s|^M||g" $file #fixes windows line endings
        echo "Appended version header to $file"
    fi

    if grep -q '@date' $file; then
        sed -i "s|@date.*|@date $date|g" $file
    fi 

    if grep -q 'Copyright (C)' $file; then
        sed -i "s|Copyright (C).*|Copyright (C) $current_year Jake Mason|g" $file
    fi
}

files=$(find ../src -type f -name '*.hpp')
for file in $files; do
    update_headers $file
done

files=$(find ../src -type f -name '*.cpp')
for file in $files; do
    update_headers $file
done


