PROJECT_ROOT="test/.."

# Ir al root del proyecto
cd "$PROJECT_ROOT" || { echo "No se pudo acceder al root del proyecto"; exit 1; }

# Compilar el proyecto
make clean || { echo "Falló 'make clean'"; exit 1; }
make build || { echo "Falló 'make build'"; exit 1; }

# Archivo de reporte
REPORT_FILE="test/test_report.txt"
echo "🧪 Reporte de tests" > "$REPORT_FILE"
echo "===================" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# Función para ejecutar un test
run_test() {
  test_name="$1"
  should_fail="$2"
  shift 2

  log_file="test/test_logs/${test_name// /_}.err"
  mkdir -p test/test_logs

  echo "🔹 Ejecutando $test_name..."

  # Ejecutar el comando: stdout va directo a pantalla, stderr se guarda y también se muestra
  "$@" 2> >(tee "$log_file" >&2)
  status=$?

  if [ "$should_fail" = "yes" ]; then
    if [ $status -ne 0 ]; then
      echo "✅ $test_name FALLÓ como se esperaba" >> "$REPORT_FILE"
          echo "🔻 Mensaje de error:" >> "$REPORT_FILE"
          tail -n 10 "$log_file" >> "$REPORT_FILE"
    else
      echo "❌ $test_name PASÓ pero se esperaba un fallo" >> "$REPORT_FILE"
      echo "🛑 Código de salida: 0 (esperado distinto de 0)" >> "$REPORT_FILE"
      echo "🔻 Últimas líneas del error:" >> "$REPORT_FILE"
      tail -n 10 "$log_file" >> "$REPORT_FILE"
      exit 1
    fi
  else
    if [ $status -eq 0 ]; then
      echo "✅ $test_name PASÓ" >> "$REPORT_FILE"
    else
      echo "❌ $test_name FALLÓ" >> "$REPORT_FILE"
      echo "🛑 Código de salida: $status (esperado 0)" >> "$REPORT_FILE"
      echo "🔻 Últimas líneas del error:" >> "$REPORT_FILE"
      tail -n 10 "$log_file" >> "$REPORT_FILE"
      exit 1
    fi
  fi

  echo "-------------------------------" >> "$REPORT_FILE"
  echo "" >> "$REPORT_FILE"
}


echo "Ejecutar tests Catedra" >> "$REPORT_FILE"
exho "" >> "$REPORT_FILE"

run_test "Test Catedra 1 (make run por defecto)" no make run

run_test "Test Catedra 2 (8 bots, sin view)" no make run BOTS=8 VIEW_ON=no

run_test "Test Catedra 3 (5 bots, Seed 124234, delay 5, timeout 15, w 13, h 12)" no make run BOTS=5 SEED=124234 DELAY=5 TIMEOUT=15 WIDTH=13 HEIGHT=12

run_test "Test Catedra 4 (9 BOTS, w = 15,  h = 15)" no make run BOTS=9 WIDTH=15 HEIGHT=15

run_test "Test Catedra 5 (w=5, h=15)" yes make run WIDTH=5 HEIGHT=15

echo "Todos los tests de la catedra pasaron correctamente." >> "$REPORT_FILE"

  echo "-------------------------------" >> "$REPORT_FILE"

echo  "Ejecutar tests Alumnos" >> "$REPORT_FILE"
exho "" >> "$REPORT_FILE"
run_test "Test Alumnos 1 (make run por defecto)" no make run_nat

run_test "Test Alumnos 2 (8 bots, sin view)" no make run_nat BOTS=8 VIEW_ON=no

run_test "Test Alumnos 3 (5 bots, Seed 124234, delay 5, timeout 15, w 13, h 30)" no make run_nat BOTS=5 SEED=124234 DELAY=10 TIMEOUT=15 WIDTH=13 HEIGHT=12

run_test "Test Alumnos 4 (9 BOTS, w = 15,  h = 15)" no make run_nat BOTS=9 WIDTH=15 HEIGHT=15

run_test "Test Catedra 5 (w=5, h=15)" yes make run_nat WIDTH=5 HEIGHT=15

run_test "Test Catedra 6 (Player: invalid)" yes ./binaries/master -p asdas

run_test "Test Catedra 7 (Player: valid, invalid)" yes ./binaries/master -p ./binaries/bot asda

run_test "Test Catedra 9 (Player: valid, invalid, View: valid)" yes ./binaries/master -p ./binaries/bot asdasd -v ./binaries/view

run_test "Test Catedra 10 (Player: valid, View: invalid)" yes ./binaries/master -p ./binaries/bot -v asdasd

run_test "Test Catedra 11 (Player: valid, invalid, View: invalid)" yes ./binaries/master -p ./binaries/bot asdasd  -v asdasd

run_test "Test Catedra 12 (Player: invalid, View: invalid)" yes ./binaries/master -p asdasd  -v asdasd


echo "Todos los tests pasaron correctamente." >> "$REPORT_FILE"