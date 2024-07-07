all:
	$(MAKE) clean
	python3 Assembler/Assembler.py
	python3 Assembler/Assembler_Evaluator.py
	cd Simulator && $(MAKE) all
	python3 logs/plots.py
clean:
	cd Simulator && $(MAKE) clean
	rm -f Output/Output.bin
	rm -f Assembler/Result/Assembler_Evaluation.txt
	rm -f Figure/*.pdf
