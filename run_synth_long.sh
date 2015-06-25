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
    if [[ "$seq" != *"No deadlock found"* ]]; then
      echo "Deadlock detection failed"
      dead="dead"
    fi
    if [[ "$con" == *"Deadlock"* ]]; then
      echo "Concurrent deadlock: " "${1}"
      dead="dead"
    elif [[ "$con" != *"No deadlock found"* ]]; then
      echo "Concurrent detection failed"
      dead="dead"
    fi
    if [ -z "$dead" ]; then
      echo "$out" | grep '|' | tr -d '\n'
    fi
}

inclusion() {
    out=$(./liss "${1}" -v 1 -inclusion -- -Itests 2>&1)
    if [[ "$out" != *"Included"* ]]; then
      echo "Inclusion check failed"
    else
      if [[ "$out" == *"Not"* ]]; then
	echo "Inclusion does not hold: " "${1}"
      fi
    fi
}

echo Compiling ...

make > /dev/null

echo Running tests ...

declare -a IGNORE=("tests/cav14/drbd_receiver.c" "tests/r8169.c")

echo "| File | Deadlock check | Threads | Iterations | max.Bound | Bug finding | Synthesis | Verification | Placement | Deadlock check2 |"

# delete complete files
find tests -name '*.complete.c' -exec rm {} \;
find tests -name '*.absmin.c' -exec rm {} \;
find tests -name '*.small.c' -exec rm {} \;
find tests -name '*.coarse.c' -exec rm {} \;
find tests -name '*.log' -exec rm {} \;

for f in tests/cav13/*.c tests/cav14/*.c tests/linux_drivers/*.c
do
  if [[ "$f" != *".locksv1.c" && "$f" != *".locksv1a.c" ]] && not_in_array IGNORE "$f"; then
    output_file1=${f/%.c/.absmin.c}
    output_file2=${f/%.c/.small.c}
    output_file3=${f/%.c/.coarse.c}
    printf "| %30s " "$f"
    # check for deadlocks
    deadlock "$f"
    if [ -z "$dead" ]; then
      out=$(./liss "$f" -v 1 -synthesis -locklimit=4 -- -Itests 2>&1 | tee "$f.log")
      if [[ "$out" == *"SANITY"* ]]; then
	echo SANITY: $f
      fi
      echo "$out" | grep '|' | tr -d '\n'
      if [[ "$out" == *"Synthesis was successful"* ]]; then
	# check an output was produced
	if [[ -e "$output_file1" && -e "$output_file2" && -e "$output_file3" ]]; then
	  # check output
	  inclusion "$output_file1"
	  inclusion "$output_file2"
	  inclusion "$output_file3"
	  deadlock "$output_file1"
	  deadlock "$output_file2"
	  deadlock "$output_file3"
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
