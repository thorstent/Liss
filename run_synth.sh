#!/bin/bash

echo Please ensure you have at least 6GB of free RAM for all examples to go through.

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

echo Compiling ...

make > /dev/null

echo Running tests ...

declare -a IGNORE=("tests/cav14/drbd_receiver.c" "tests/r8169.c")

echo "| File | Threads | Iterations | max.Bound | Bug finding | Synthesis | Verification | Total |"

# delete complete files
find tests -name '*.complete.c' -exec rm {} \;
find tests -name '*.absmin.c' -exec rm {} \;
find tests -name '*.small.c' -exec rm {} \;
find tests -name '*.coarse.c' -exec rm {} \;
find tests -name '*.unopt.c' -exec rm {} \;
find tests -name '*.log' -exec rm {} \;

for f in tests/cav13/*.c tests/cav14/*.c tests/linux_drivers/*.c
do
  if [[ "$f" != *".locksv1.c" && "$f" != *".locksv1a.c" ]] && not_in_array IGNORE "$f"; then
    output_file=${f/%.c/.complete.c}
    printf "| %30s " "$f |"
    # check for deadlocks
    if [ -z "$dead" ]; then
      out=$(./liss "$f" -v 1 -synthesis -- -Itests 2>&1 | tee "$f.log")
      if [[ "$out" == *"SANITY"* ]]; then
	echo SANITY: $f
      fi
      echo "$out" | grep '|' | tr -d '\n'
      if [[ "$out" == *"Found no valid lock placement"* ]]; then
	echo Failed lock placement
      fi
      if [[ "$out" != *"Synthesis was successful"* ]]; then
	# check an output was produced
	echo Failed synth
      fi
    fi
    echo
  fi
done

echo Done
