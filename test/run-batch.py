
'''
#input: l1, l2 [instance1, instance2, instance3, .....]
#output: output.csv
'''

import argparse
import time
import numpy
import multiprocessing
import subprocess
import shlex
import re


# Config path
app_path = "/home/luan/project/IVMP/"
instances_path = "/home/luan/project/IVMP/instances_new/"
output_path = "/home/luan/project/IVMP/output/gupta_conf4/"

input_inst = ""

verbose = False
write_flag = True
combination = False


def read_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('--inst', nargs='+')
    parser.add_argument('--input_inst', default='instances.txt')
    parser.add_argument('--repeat', nargs='?',  type=int)
    parser.add_argument('--gupta', help='Print more data',  action='store_true')
    parser.add_argument('--gupta_comb', help='Print more data',  action='store_true')
    parser.add_argument('--ils', help='Print more data',  action='store_true')
    parser.add_argument('--write', help='write output in a file', action='store_true')
    parser.add_argument('--verbose', help='Print more data', action='store_true')
    parser.add_argument('--output_path', default=".")

    global verbose, write_flag, output_path, combination, input_inst

    instances = parser.parse_args().inst
    repeat = parser.parse_args().repeat
    gupta_flag = parser.parse_args().gupta
    ils_flag = parser.parse_args().ils

    write_flag = parser.parse_args().write

    verbose = parser.parse_args().verbose
    output_path = parser.parse_args().output_path
    combination = parser.parse_args().gupta_comb

    input_inst = parser.parse_args().input_inst

    # print verbose, write_flag, output_path

    if repeat is None:
        repeat = 1

    return instances, repeat, gupta_flag, ils_flag


def write_file(output, path, file_name):
    # print path + file_name
    with open(path + file_name, "w") as f:
        f.write(output)


def prepare_cmd(instance, parameters=[], algorithm='i'):
    return app_path + "interference " + instance + "  " + algorithm


def exec_command(cmd):
    p = None

    try:
        p = subprocess.Popen(shlex.split(cmd),
                stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

    except:
        print "error"

    return p


def top_msg(msg):
    out = 20 * "#"
    out += "\n\t"+msg + "\n"
    out += 20 * "#"
    return out


def print_warning(msg):
    if verbose:
        print msg


def exec_gupta(instances, l1=None, l2=None):
    instances.sort()

    out_file = ""
    for instance in instances:
        output, error = exec_command(prepare_cmd(instance, algorithm="g"))

        print_warning(top_msg("Output"))
        print_warning(output)

        if write_flag:
            with open(output_path+instance+"_out", "w") as f:
                f.write(output)

        num_machines = int(output.splitlines()[4].split()[-1])
        interference = float(output.splitlines()[6].split()[-1])

        print_warning(instance + ", " + str(interference) + ", " + str(num_machines))
        out_file += instance + ", " + str(interference) + ", " + str(num_machines) + "\n"

    write_file(out_file, "", "gupta.csv")


def exec_ils(instances, repeat):
    instances.sort()
    out_file = ""
    for instance in instances:
        interference = []
        machines = []
        for i in range(repeat):
            output, error = exec_command(prepare_cmd(instance, algorithm="i"))
            time.sleep(1)
            if output:
                print_warning(top_msg("Output"))
                print_warning(output)
            else:
                print_warning(top_msg("Output"))
                print_warning(error)
            if write_flag:
                with open(output_path+instance+"_out", "a") as f:
                    f.write(output + "\n")
            #  Computa a media
            machines.append(int(output.splitlines()[4].split()[-1]))
            interference.append(float(output.splitlines()[6].split()[-1]))
        # statistic
        avg_interference = numpy.mean(interference)
        avg_numMachine = numpy.mean(machines)
        std_interference = numpy.std(interference)
        std_machines = numpy.std(machines)
        if write_flag:
                with open(output_path+instance+"_out", "a") as f:
                    f.write("\n" + str(avg_interference) + "  +-" + str(std_interference) + " " + str(avg_numMachine) + "\n")
        print_warning(instance + ", " + str(avg_interference) + ", " + str(avg_numMachine))
        out_file += instance + ", " + str(avg_interference) + ", " + str(std_interference) + ", " + str(avg_numMachine) + "\n"
    write_file(out_file, "", "ils.csv")


def exec_gupta_combinations(instances):
    pass


def exec_dmer(repeat, i_file, i_maxiter, i_maxtime, i_target):
    # Defining the programs to run
    versions = []
    versions.append("sequential")
    versions.append("mpi-lb")
    versions.append("mpi-pthread-lb")
    versions.append("mpi-openmp-lb")
    versions.append("mpi")
    versions.append("mpi-pthread")
    versions.append("mpi-openmp")
    exec_batch(versions, repeat, file=i_file, maxiter=i_maxiter, maxtime=i_maxtime, target=i_target)


def exec_batch(versions, repeat, file="danoint.mps.g.g", maxiter=100, maxtime=30000, target=96):
    out_file = ""
    for version in versions:
        with open("./dmer_executions.csv", "a") as f:
            f.write(version + "\n")
        exec_times = []
        exec_times_ttt = []
        command = "./run-" + version + ".sh"
        command += " ./input/" + str(file) + ".mps.g.g"
        command += " " + str(maxiter)
        command += " " + str(maxtime)
        command += " " + str(target)
        print command
        for i in range(repeat):
            # print version + " " + str(i)
            output, error = exec_command(command)
            # print output
            m = re.search("[0-9]*,([0-9]*[.][0-9]*),[0-9]*,([0-9]*[.][0-9]*)", output, re.M)
            # print m.group(0)
            # print m.group(1)
            # print m.group(2)
            # print m
            exec_times.append(float(m.group(1)))
            exec_times_ttt.append(float(m.group(2)))
            # with open(output_path + "/error_output", "w") as f : f.write(error)
            # output, error = exec_command("awk '/^[0-9]*.[0-9]*$/{print $1};' error_output")
            # exec_times.append(float(output.split()[0]))
            with open("./dmer_executions.csv", "a") as f:
                f.write(output)
            # print output
            # if (i == 0):
            #     with open("./dmer_executions.csv", "a") as f:
            #         f.write(version + " (PxT=" + str(np)
            #                 + "x" + str(nt) + ")," + str(exec_times[i]))
            # else:
            #     with open("./kirchhoff_executions.csv", "a") as f:
            #         f.write("," + str(exec_times[i]))
        # with open("./kirchhoff_executions.csv", "a") as f:
        #     f.write("\n")
        avg_exec_times = numpy.mean(exec_times)
        std_exec_times = numpy.std(exec_times)
        with open("./dmer_executions.csv", "a") as f:
            f.write(str(avg_exec_times) + "," + str(std_exec_times) + "\n")
        avg_exec_times = numpy.mean(exec_times_ttt)
        std_exec_times = numpy.std(exec_times_ttt)
        with open("./dmer_executions.csv", "a") as f:
            f.write(str(avg_exec_times) + "," + str(std_exec_times) + "\n")
        # print "Average of execution times: " + str(avg_exec_times)
        # print "Standart deviation: " + str(std_exec_times)
        # out_file += version + ", " + str(avg_exec_times) + ", " + str(std_exec_times) + "\n"
    # write_file(out_file, "", "./kirchhoff.csv")
    # with open("./kirchhoff.csv", "a") as f:
    #     f.write(out_file)

# --- Main Program ---


versions = []
instances, repeat, gupta_flag, ils_flag = read_args()

# output, error = exec_command("/opt/intel/parallel_studio_xe_2018/bin/psxevars.sh")

# Cleaning files
with open(output_path + "/dmer_executions.csv", "w") as f:
    f.write("")
# with open(output_path + "/kirchhoff.csv", "w") as f:
#     f.write("")

# Execution in batch
# with open("./kirchhoff_executions.csv", "a") as f : f.write("NFC=8, FWIDTH=5\n")
# with open("./kirchhoff.csv", "a") as f : f.write("NFC=8, FWIDTH=5\n")
# exec_sequential(repeat, 8, 5)
# exec_parallel_omp(repeat, 8, 5, 2)
# exec_parallel_mpi(repeat, 8, 5, 2)
# exec_parallel_omp(repeat, 8, 5, 3)
# exec_parallel_mpi(repeat, 8, 5, 3)
# exec_parallel_omp(repeat, 8, 5, 4)
# exec_parallel_mpi(repeat, 8, 5, 4)
# exec_parallel_omp(repeat, 8, 5, 5)
# exec_parallel_mpi(repeat, 8, 5, 5)
# exec_parallel_omp(repeat, 8, 5, 6)
# exec_parallel_mpi(repeat, 8, 5, 6)
# exec_hybrid(repeat, 8, 5, 2, 3)
# exec_hybrid(repeat, 8, 5, 3, 2)

# with open("./kirchhoff_executions.csv", "a") as f : f.write("NFC=16, FWIDTH=3\n")
# with open("./kirchhoff.csv", "a") as f : f.write("NFC=16, FWIDTH=3\n")
# exec_sequential(repeat, 16, 3)
# exec_parallel_omp(repeat, 16, 3, 2)
# exec_parallel_mpi(repeat, 16, 3, 2)
# exec_parallel_omp(repeat, 16, 3, 3)
# exec_parallel_mpi(repeat, 16, 3, 3)
# exec_parallel_omp(repeat, 16, 3, 4)
# exec_parallel_mpi(repeat, 16, 3, 4)
# exec_parallel_omp(repeat, 16, 3, 5)
# exec_parallel_mpi(repeat, 16, 3, 5)
# exec_parallel_omp(repeat, 16, 3, 6)
# exec_parallel_mpi(repeat, 16, 3, 6)
# exec_hybrid(repeat, 16, 3, 2, 3)
# exec_hybrid(repeat, 16, 3, 3, 2)

# with open("./dmer_executions.csv", "a") as f:
#     f.write("NFC=32, FWIDTH=1\n")
# with open("./kirchhoff.csv", "a") as f:
#     f.write("NFC=32, FWIDTH=1\n")

with open("./dmer_executions.csv", "a") as f:
    f.write("danoint.mps.g.g\n")
exec_dmer(repeat, "danoint.mps.g.g", 100, 2147483647, 96)

with open("./dmer_executions.csv", "a") as f:
    f.write("bienst1\n")
exec_dmer(repeat, "bienst1", 100, 2147483647, 90)

with open("./dmer_executions.csv", "a") as f:
    f.write("1-t0.300-to0.05-y2003-m11-p90-signed\n")
exec_dmer(repeat, "1-t0.300-to0.05-y2003-m11-p90-signed", 100, 2147483647, 336)

with open("./dmer_executions.csv", "a") as f:
    f.write("r80x800\n")
exec_dmer(repeat, "r80x800", 100, 2147483647, 113)

with open("./dmer_executions.csv", "a") as f:
    f.write("neos21\n")
exec_dmer(repeat, "neos21", 100, 2147483647, 587)

with open("./dmer_executions.csv", "a") as f:
    f.write("n12-3\n")
exec_dmer(repeat, "n12-3", 100, 2147483647, 1273)

# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)

# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)

# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)
# exec_dmer(repeat, "danoint.mps.g.g", 100, 30000, 96)

# exec_sequential(repeat, 32, 1)
# exec_parallel_omp(repeat, 32, 1, 2)
# exec_parallel_mpi(repeat, 32, 1, 2)
# exec_parallel_omp(repeat, 32, 1, 3)
# exec_parallel_mpi(repeat, 32, 1, 3)
# exec_parallel_omp(repeat, 32, 1, 4)
# exec_parallel_mpi(repeat, 32, 1, 4)
# exec_parallel_omp(repeat, 32, 1, 5)
# exec_parallel_mpi(repeat, 32, 1, 5)
# exec_parallel_omp(repeat, 32, 1, 6)
# exec_parallel_mpi(repeat, 32, 1, 6)
# exec_hybrid(repeat, 32, 1, 2, 3)
# exec_hybrid(repeat, 32, 1, 3, 2)

# exec_parallel_omp(repeat, 32, 1, 10)
# exec_parallel_mpi(repeat, 32, 1, 10)

# exec_parallel_omp(repeat, 32, 1, 20)
# exec_parallel_mpi(repeat, 32, 1, 20)

# exec_parallel_omp(repeat, 32, 1, 30)
# exec_parallel_mpi(repeat, 32, 1, 30)

# exec_parallel_omp(repeat, 32, 1, 40)
# exec_parallel_mpi(repeat, 32, 1, 40)

# exec_batch(versions, repeat)
