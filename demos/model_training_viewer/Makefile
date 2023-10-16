build_cpp:
	ln -s $(CURDIR)/../../build ./build_cpp

.venv:
	python3 -m venv .venv
	.venv/bin/pip install -r requirements.txt

data: .venv
	.venv/bin/python ./script/get_data.py

src/py/realtime:
	ln -s $(CURDIR)/../../src/py/realtime ./src/py/realtime

src/rs: src
	ln -s $(CURDIR)/../../src/rs ./src/rs

src/js/.env:
	@if [ -z "$(RECEIVER_WEBSOCKET_URL)" ]; then echo "RECEIVER_WEBSOCKET_URL is not set" && exit 1; fi
	echo RECEIVER_WEBSOCKET_URL=$(RECEIVER_WEBSOCKET_URL) > ./src/js/.env
