#!/bin/bash

not_in_array() {
    local haystack=${1}[@]
    local needle=${2}
    for i in ${!haystack}; do
        if [[ ${i} == ${needle} ]]; then
            return 1
        fi
    done
    return 0
}

deadlock() {
    out=$(./liss "${1}" -v 1 -deadlock -- -Itests 2>&1 )
    seq=$(echo "$out" | grep Sequential)
    con=$(echo "$out" | grep Concurrent)
    dead=""
    if [[ "$seq" == *"Deadlock"* ]]; then
      echo "Sequential deadlock"
      dead="dead"
    elif [[ "$seq" != *"No deadlock found"* ]]; then
      echo "Deadlock detection failed"
      dead="dead"
    fi
    if [[ "$con" == *"Deadlock"* ]]; then
      echo "Concurrent deadlock"
      dead="dead"
    elif [[ "$con" != *"No deadlock found"* ]]; then
      echo "Concurrent detection failed"
      dead="dead"
    fi
    if [ -z "$dead" ]; then
      echo "$out" | grep '|' | tr -d '\n'
    fi
}

echo Compiling ...

make > /dev/null

echo Running tests ...

declare -a IGNORE=("tests/cav14/drbd_receiver.c" "tests/r8169.c")

echo "| File | Deadlock check | Threads | Iterations | max.Bound | Bug finding | Synthesis | Verification | Placement | Deadlock check2 |"

# delete complete files
find tests -name '*.complete.c' -exec rm {} \;

for f in tests/*.c tests/cav13/*.c tests/cav14/*.c tests/linux_drivers/*.c
do
  if [[ "$f" != *".complete.c" ]] && not_in_array IGNORE "$f"; then
    output_file=${f/%.c/.complete.c}
    printf "| %30s " "$f"
    # check for deadlocks
    deadlock "$f"
    if [ -z "$dead" ]; then
      out=$(./liss "$f" -v 1 -synthesis -- -Itests 2>&1 | tee "$f.log")
      if [[ "$out" == *"SANITY"* ]]; then
	echo SANITY: $f
      fi
      if [[ "$out" == *"Synthesis was successful"* ]]; then
	echo "$out" | grep '|' | tr -d '\n'
	# check an output was produced
	if [ -e "$output_file" ]; then
	  # check output
	  out=$(./liss "$output_file" -v 1 -inclusion -- -Itests 2>&1)
	  if [[ "$out" != *"Included"* ]]; then
	    echo "Inclusion check failed"
	  else
	    if [[ "$out" == *"Not"* ]]; then
	      echo "Inclusion does not hold"
	    fi
	  fi
	  deadlock "$output_file"
	else
	  echo "No output produced"
	fi
      else
	echo Incorrect output
      fi
      if [[ "$out" == *"program incomplete"* ]]; then
	echo Failed
      fi
    fi
    echo
  fi
done

echo Done
