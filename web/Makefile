all:
	emcc *.c -s ASYNCIFY=1 -s -o test.html
	python3 -m http.server 5000
