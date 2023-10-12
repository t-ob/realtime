build_cpp:
	@if [ -z "$(CMAKE_TOOLCHAIN_FILE)" ]; then echo "CMAKE_TOOLCHAIN_FILE is not set" && exit 1; fi
	@if [ -z "$(BOOST_ROOT)" ]; then echo "BOOST_ROOT is not set" && exit 1; fi
	cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_TOOLCHAIN_FILE:STRING=$(CMAKE_TOOLCHAIN_FILE) -DBOOST_ROOT:STRING=$(BOOST_ROOT) -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ -S../../src/cpp -Bbuild_cpp/Debug -G "Unix Makefiles"
	cmake --build build_cpp/Debug --config Debug --target all -j 26 -- 2>&1

build_wasm:
	wasm-pack build ../../src/rs/receiver --out-dir $(CURDIR)/build_wasm --target web

.venv:
	python3 -m venv .venv
	.venv/bin/pip install -r requirements.txt

data: .venv
	.venv/bin/python ./script/get_data.py

src:
	mkdir src

src/py/realtime:
	mkdir -p src/py/realtime

src/py/realtime/__init__.py: src/py/realtime
	cp ../../src/py/realtime/__init__.py src/py/realtime/

src/rs: src
	ln -s $(CURDIR)/../../src/rs ./src/rs
