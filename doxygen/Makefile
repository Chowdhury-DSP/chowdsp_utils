.PHONY: all deploy

all:
	doxygen Doxyfile

deploy: all
	scp -r ../docs jatin@ccrma-gate.stanford.edu:~/Library/Web/chowdsp/chowdsp_utils
