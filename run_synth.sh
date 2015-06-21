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

#echo Compiling ...

#make -C build/buildr > /dev/null

echo Running tests ...

declare -a IGNORE=("tests/cav14/drbd_receiver.c" "tests/r8169.c")

echo "| File | Threads | Iterations | max.Bound | Bug finding | Synthesis | Verification | Max Mem. |"

for f in tests/cav13/*.c tests/cav14/*.c tests/linux_drivers/*.c
do
  if [[ "$f" != *".complete.c" && "$f" != *".start.c" ]] && not_in_array IGNORE "$f"; then
    printf "| %30s " "$f"
    out=$(./liss "$f" -v 1 -synthesis -- -Itests 2>&1 | tee "$f.log")
    if [[ "$out" == *"SANITY"* ]]; then
      echo SANITY: $f
    fi
    if [[ "$out" == *"Synthesis was successful"* ]]; then
      #echo -n Success
      echo "$out" | grep '|'
    fi
    if [[ "$out" == *"program incomplete"* ]]; then
      echo Failed
    fi
    

  fi
done

echo Done
