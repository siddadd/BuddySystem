# This python script checks line by line the memory allocation made by the developed Buddy System with the expected ouput of the Buddy System
# SPRING 2013

import sys
import subprocess
import difflib

RUN_ALL = 0
RUN_1 = 0
RUN_2 = 0
RUN_3 = 0
RUN_4 = 1
RUN_5 = 1

def test_buddy(command, gen_output_file, ref_output_file):
	
	print("Running Buddy System:" + command)

	subprocess.check_call("./" + command)

	fp_gen = open(gen_output_file)
	gen_lines = fp_gen.readlines()
	#print(gen_lines)
	fp_gen.close()
	#print("##########")
	fp_ref = open(ref_output_file)
	ref_lines = fp_ref.readlines()
	#print(ref_lines)
	fp_ref.close()

	d = difflib.Differ()
	matcher = difflib.SequenceMatcher(None, ref_lines, gen_lines)
	
	if matcher.ratio() == 1.0:
		print "All is Well"
	else:
		print "Error: Check error.log"
		result = d.compare(ref_lines, gen_lines)
		fp_log = open("error.log", "a")
		fp_log.write("Error:" + command + "\n")
		fp_log.writelines(result)
		fp_log.close()

def run_me(args=None):
	if RUN_ALL or RUN_1:
		test_buddy("test1_out", "test1_output.txt", "ref_test1_output.txt")
	if RUN_ALL or RUN_2:
		test_buddy("test2_out", "test2_output.txt", "ref_test2_output.txt")
	if RUN_ALL or RUN_3:
		test_buddy("test3_out", "test3_output.txt", "ref_test3_output.txt")
	if RUN_ALL or RUN_4:
		test_buddy("test4_out", "test4_output.txt", "ref_test4_output.txt")
	if RUN_ALL or RUN_5:
		test_buddy("test5_out", "test5_output.txt", "ref_test5_output.txt")

if __name__ == "__main__":
	run_me()
