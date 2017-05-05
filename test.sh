#!/bin/bash
FAIL=0

# make a fresh copy of the target program
make clean
make
if [ $? -ne 0 ]; then
  echo "**** Make (compilation) FAILED"
  FAIL=1
fi

# Function to run the program against a (successful) test case.
runtest() {
  TESTNO=$1
  ESTATUS=$2

  rm -f output.txt stderr.txt

  echo "Test $TEST_NO: ./interpreter prog_$TESTNO.txt > output.txt 2> stderr.txt"
  ./interpreter prog_$TESTNO.txt > output.txt 2> stderr.txt
  STATUS=$?

  # Make sure the program exited with the right exit status.
  if [ $STATUS -ne $ESTATUS ]; then
      echo "**** Test failed - incorrect exit status. Expected: $ESTATUS Got: $STATUS"
      FAIL=1
      return 1
  fi

  # Make sure the output matches the expected output.
  diff -q expected_$TESTNO.txt output.txt >/dev/null 2>&1
  if [ $? -ne 0 ]; then
      echo "**** Test FAILED - output doesn't match expected"
      FAIL=1
      return 1
  fi

  # Make sure the error matches expected.
  diff -q stderr_$TESTNO.txt stderr.txt >/dev/null 2>&1
  if [ $? -ne 0 ]; then
      echo "**** Test FAILED - error output doesn't match expected"
      FAIL=1
      return 1
  fi

  echo "Test $TESTNO PASS"
  return 0
}

# Run successfule test cases
runtest 01 0
runtest 02 0
runtest 03 0
runtest 04 0
runtest 05 0
runtest 06 0
runtest 07 0
runtest 08 0
runtest 09 0
runtest 10 0
runtest 11 0
runtest 12 0 
runtest 13 0
runtest 14 0
runtest 15 0
runtest 16 0
runtest 17 1
runtest 18 1
runtest 19 1
runtest 20 1

if [ $FAIL -ne 0 ]; then
  echo "FAILING TESTS!"
  exit 13
else 
  exit 0
fi
