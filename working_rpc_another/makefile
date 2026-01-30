# Глобальные переменные
CC = gcc
CFLAGS = -g -Wall -I/usr/include/tirpc
LDFLAGS = -ltirpc -lpthread -lm
RPCGEN = rpcgen

# Директории
CALC_DIR = calculator
BAKERY_DIR = bakery

# Цели по умолчанию
all: calculator bakery

# =============================================
# КАЛЬКУЛЯТОР
# =============================================

CALC_X = $(CALC_DIR)/calculator.x
CALC_GEN = $(CALC_DIR)/calculator.h $(CALC_DIR)/calculator_clnt.c \
           $(CALC_DIR)/calculator_svc.c $(CALC_DIR)/calculator_xdr.c

# Генерация RPC файлов для калькулятора
$(CALC_GEN): $(CALC_X)
	@echo "Generating RPC files for Calculator..."
	cd $(CALC_DIR) && $(RPCGEN) -a calculator.x
	@echo "Note: Modify Makefile.calculator if needed"

# Сборка калькулятора с использованием сгенерированного Makefile
calculator: $(CALC_GEN)
	@echo "Building Calculator..."
	cd $(CALC_DIR) && $(MAKE) -f Makefile.calculator calculator_server calculator_client
	@cp $(CALC_DIR)/calculator_server $(CALC_DIR)/calculator_client .

# =============================================
# БУЛОЧНАЯ
# =============================================

BAKERY_X = $(BAKERY_DIR)/bakery.x
BAKERY_GEN = $(BAKERY_DIR)/bakery.h $(BAKERY_DIR)/bakery_clnt.c \
             $(BAKERY_DIR)/bakery_svc.c $(BAKERY_DIR)/bakery_xdr.c

# Генерация RPC файлов для булочной
$(BAKERY_GEN): $(BAKERY_X)
	@echo "Generating RPC files for Bakery..."
	cd $(BAKERY_DIR) && $(RPCGEN) -a bakery.x
	@echo "Note: You need to create Makefile.bakery or modify manually"

# Ручная сборка булочной
bakery: $(BAKERY_GEN)
	@echo "Building Bakery (manual compilation)..."
	$(CC) $(CFLAGS) -c $(BAKERY_DIR)/bakery_clnt.c -o $(BAKERY_DIR)/bakery_clnt.o
	$(CC) $(CFLAGS) -c $(BAKERY_DIR)/bakery_xdr.c -o $(BAKERY_DIR)/bakery_xdr.o
	$(CC) $(CFLAGS) -c $(BAKERY_DIR)/bakery_client.c -o $(BAKERY_DIR)/bakery_client.o
	$(CC) $(CFLAGS) -o bakery_client $(BAKERY_DIR)/bakery_client.o \
		$(BAKERY_DIR)/bakery_clnt.o $(BAKERY_DIR)/bakery_xdr.o $(LDFLAGS)
	
	$(CC) $(CFLAGS) -c $(BAKERY_DIR)/bakery_svc.c -o $(BAKERY_DIR)/bakery_svc.o
	$(CC) $(CFLAGS) -c $(BAKERY_DIR)/bakery_server.c -o $(BAKERY_DIR)/bakery_server.o
	$(CC) $(CFLAGS) -o bakery_server $(BAKERY_DIR)/bakery_server.o \
		$(BAKERY_DIR)/bakery_svc.o $(BAKERY_DIR)/bakery_xdr.o $(LDFLAGS)

# =============================================
# СЛУЖЕБНЫЕ ЦЕЛИ
# =============================================

clean:
	@echo "Cleaning..."
	rm -f calculator_server calculator_client bakery_server bakery_client
	rm -f $(CALC_DIR)/*.o $(BAKERY_DIR)/*.o
	rm -f $(CALC_DIR)/calculator_* $(CALC_DIR)/Makefile.calculator
	rm -f $(BAKERY_DIR)/bakery_* $(BAKERY_DIR)/Makefile.bakery
	rm -f *~ */*~ core

clean-all: clean
	rm -f $(CALC_GEN) $(BAKERY_GEN)

# Цели для запуска
run-calculator: calculator_server
	@echo "Starting Calculator Server..."
	./calculator_server &

run-bakery: bakery_server
	@echo "Starting Bakery Server..."
	./bakery_server &

test-calculator: calculator_client
	@echo "Testing Calculator Client..."
	./calculator_client localhost

test-bakery: bakery_client
	@echo "Testing Bakery Client..."
	./bakery_client localhost

test-calculator-multi: calculator_client
	@echo "Testing Multithreaded Calculator Client..."
	./calculator_client localhost --multi

stop-servers:
	@echo "Stopping all servers..."
	-killall calculator_server
	-killall bakery_server

.PHONY: all calculator bakery clean clean-all \
        run-calculator run-bakery test-calculator test-bakery \
        test-calculator-multi stop-servers

calc_start_server:
	./$(CALC_DIR)/calculator_server

calc_stop_server:
	killall calculator_server

calc_start_client:
	./$(CALC_DIR)/calculator_client localhost

bakery_start_server:
	./$(BAKERY_DIR)/bakery_server

bakery_stop_server:
	killall bakery_server

bakery_start_client:
	./$(BAKERY_DIR)/bakery_client localhost
