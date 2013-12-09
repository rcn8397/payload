show: a_better_udp.pdf
	acroread $^

a_better_udp.pdf: a_better_udp.tex
	pdflatex a_better_udp
	bibtex   a_better_udp
	pdflatex a_better_udp
	pdflatex a_better_udp

clean:
	-rm a_better_udp.log
	-rm a_better_udp.aux
	-rm a_better_udp.bbl
	-rm a_better_udp.blg

clobber:
	-rm a_better_udp.pdf
