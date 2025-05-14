PROJECT_ROOT="../"

# Go to project root
cd "$PROJECT_ROOT" || { echo "Could not access project root"; exit 1; }

# Compile the project
make clean || { echo "Failed 'make clean'"; exit 1; }
make build || { echo "Failed 'make build'"; exit 1; }

# Report file
REPORT_FILE="test/test_report.txt"
echo "Test Report" > "$REPORT_FILE"
echo "==============" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# Function to run a test
run_test() {
  test_name="$1"
  should_fail="$2"
  shift 2

  log_file="test/test_logs/${test_name// /_}.err"
  mkdir -p test/test_logs

  echo "🔹 Running $test_name..."

  # Run command: stdout to terminal, stderr to log (and also shown)
  "$@" 2> >(tee "$log_file" >&2)
  status=$?

  if [ "$should_fail" = "yes" ]; then
    if [ $status -ne 0 ]; then
      echo "$test_name FAILED as expected" >> "$REPORT_FILE"
      echo "Error message:" >> "$REPORT_FILE"
      tail -n 10 "$log_file" >> "$REPORT_FILE"
    else
      echo "$test_name PASSED but failure was expected" >> "$REPORT_FILE"
      echo "Exit code: 0 (expected non-zero)" >> "$REPORT_FILE"
      echo "Last lines of stderr:" >> "$REPORT_FILE"
      tail -n 10 "$log_file" >> "$REPORT_FILE"
      exit 1
    fi
  else
    if [ $status -eq 0 ]; then
      echo "$test_name PASSED" >> "$REPORT_FILE"
    else
      echo "$test_name FAILED" >> "$REPORT_FILE"
      echo "Exit code: $status (expected 0)" >> "$REPORT_FILE"
      echo "Last lines of stderr:" >> "$REPORT_FILE"
      tail -n 10 "$log_file" >> "$REPORT_FILE"
      exit 1
    fi
  fi

  echo "-------------------------------" >> "$REPORT_FILE"
  echo "" >> "$REPORT_FILE"
}

echo "Running Catedra tests" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

run_test "Test Catedra 1 (default make run)" no make run VIEW_ON=yes

run_test "Test Catedra 2 (8 bots, no view)" no make run BOTS=8 

run_test "Test Catedra 3 (5 bots, Seed 124234, delay 5, timeout 15, w 13, h 12)" no make run BOTS=5 SEED=124234 DELAY=100 TIMEOUT=15 WIDTH=13 HEIGHT=12 VIEW_ON=yes

run_test "Test Catedra 4 (9 BOTS, w = 15,  h = 15)" no make run BOTS=9 WIDTH=15 HEIGHT=15 VIEW_ON=yes

run_test "Test Catedra 5 (w=5, h=15)" yes make run WIDTH=5 HEIGHT=15 VIEW_ON=yes

echo "All Catedra tests passed successfully." >> "$REPORT_FILE"
echo "-------------------------------" >> "$REPORT_FILE"

echo "Running Student tests" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

run_test "Test Student 1 (default make run)" no make run_nat VIEW_ON=yes

run_test "Test Student 2 (8 bots, no view)" no make run_nat BOTS=8

run_test "Test Student 3 (5 bots, Seed 124234, delay 5, timeout 15, w 13, h 30)" no make run_nat BOTS=5 SEED=124234 DELAY=100 TIMEOUT=15 WIDTH=13 HEIGHT=12 VIEW_ON=yes

run_test "Test Student 4 (9 BOTS, w = 15,  h = 15)" no make run_nat BOTS=9 WIDTH=15 HEIGHT=15 VIEW_ON=yes

# run_test "Test Catedra 5 (w=5, h=15)" yes make run_nat WIDTH=5 HEIGHT=15 VIEW_ON=yes

# run_test "Test Catedra 6 (Player: invalid)" yes ./binaries/master -p asdas

# run_test "Test Catedra 7 (Player: valid, invalid)" yes ./binaries/master -p ./binaries/bot asda

# run_test "Test Catedra 9 (Player: valid, invalid, View: valid)" yes ./binaries/master -p ./binaries/bot asdasd -v ./binaries/view

# run_test "Test Catedra 10 (Player: valid, View: invalid)" yes ./binaries/master -p ./binaries/bot -v asdasd

# run_test "Test Catedra 11 (Player: valid, invalid, View: invalid)" yes ./binaries/master -p ./binaries/bot asdasd  -v asdasd

# run_test "Test Catedra 12 (Player: invalid, View: invalid)" yes ./binaries/master -p asdasd  -v asdasd

echo "All tests passed successfully." >> "$REPORT_FILE"
