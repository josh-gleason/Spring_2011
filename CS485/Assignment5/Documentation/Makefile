Report.pdf: Report.tex head.tex body.tex title.tex abstract.tex title.tex logo.png images.tex
	pdflatex Report.tex
	pdflatex Report.tex
	echo `rm Report.aux Report.toc Report.log Report.out Report.lof`
	okular Report.pdf

clean:
	rm Report.pdf

.PHONY: clean Report.pdf
