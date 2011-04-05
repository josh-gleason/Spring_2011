all: jeremiah.pdf solutions.pdf

jeremiah.pdf: jeremiah.tex
	pdflatex jeremiah.tex
	pdflatex jeremiah.tex
	rm jeremiah.aux jeremiah.log
	evince jeremiah.pdf

solutions.pdf: solutions.tex
	pdflatex solutions.tex
	pdflatex solutions.tex
	rm solutions.aux solutions.log
	evince solutions.pdf

.PHONY: jeremiah.pdf solutions.pdf
