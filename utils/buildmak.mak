.PHONY: all

all : $(sort $(subst .s,.S,$(wildcard *.c *.cc *.s)))
	@echo.exe $^
