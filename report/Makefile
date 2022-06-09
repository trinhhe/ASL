all: report.pdf

imgs = $(filter-out img/counterexamples.pdf, $(patsubst %.asy,%.pdf,$(wildcard img/*.asy)) $(patsubst %.epdf,%.pdf,$(wildcard img/*.epdf)))

define build-once
	pdflatex -interaction nonstopmode -file-line-error $<
	cp report.pdf out.pdf
	bibtex report || true
endef

# LaTeX must be run multiple times to get references right
report.pdf: report.tex $(wildcard *.tex) $(wildcard macros/*.tex) bibliography.bib spconf.sty
	$(call build-once)
	$(call build-once)

clean:
	rm -f *.log *.dvi *.aux *.toc *.lof *.lot *.out *.bbl *.blg *.xmpi *.bcf *.run.xml *.syntex.gz
	rm -f report.pdf
