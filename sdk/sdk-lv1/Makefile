LIB_VERSION = 0.0.0
export LIB_VERSION

Target = ./src/lib		\
#	./src/tools		\
	./src/ko/jz_sys		\
	./src/ko/jz_venc

#all: doc modules pdf
all: modules

modules:
	@$(foreach m, $(Target), $(MAKE) -C $(m);)

doc:
	@cd doc; doxygen Doxyfile

pdf:
	@cd doc/latex; make > /dev/null

test:
	@$(MAKE) -C ./test

clean:
	@$(foreach m, $(Target), $(MAKE) -C $(m) clean;)
	@$(MAKE) -C ./test clean
	@rm ./doc/html -rf
	@rm ./doc/latex -rf

install:
	@$(foreach m, $(Target), $(MAKE) -s -C $(m) install;)
	@mkdir -p ./out/include
	@cp ./include/* ./out/include
#	@mkdir -p ./out/doc-html
#	@cp -r ./doc/html/* ./out/doc-html
#	@cp -r ./doc/latex/refman.pdf ./out/sdk.pdf

distclean: clean
	@rm ./out -rf
	@$(MAKE) -C ./test distclean

.PHONY: doc test
