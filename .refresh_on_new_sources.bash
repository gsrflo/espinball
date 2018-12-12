#!/bin/bash

CURRENT_SOURCE_DIR=$1


if [[ ! -e list_sourcefiles ]];
then
	ls ${CURRENT_SOURCE_DIR}/code/ | grep "\.c" | tr ' ' '\n' > list_sourcefiles;
fi;

new_files=(`ls ${CURRENT_SOURCE_DIR}/code | grep "\.c" | tr ' ' '\n'`);
stored_files=(`cat list_sourcefiles`);

contents_match=1;
if [[ ${#new_files[@]} -eq ${#stored_files[@]} ]]; then
	for index in ${!new_files[@]}; do
		if [[ ${new_files[${index}]} != ${stored_files[${index}]} ]]; then
			contents_match=0;
			continue;
		fi;
	done;

	if  [[ contents_match -eq 1 ]]; then
		exit 0;
	fi;
fi;


ls ${CURRENT_SOURCE_DIR}/code/ | grep "\.c" | tr ' ' '\n' > list_sourcefiles;

cmake ${CURRENT_SOURCE_DIR};
