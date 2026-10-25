import os
import shlex
import subprocess
import sys
import termios
import tty


def info(message):
	print(f"\033[34m{message}\033[0m")


def warn(message):
	print(f"\033[33m{message}\033[0m")


def error(message):
	print(f"\033[31m{message}\033[0m")


def choose(prompt, choices):
	fd = sys.stdin.fileno()
	settings = termios.tcgetattr(fd)
	print(prompt, end="", flush=True)
	try:
		# cbreak mode disables line buffering and terminal echo while keeping Ctrl+C enabled.
		tty.setcbreak(fd)
		while True:
			choice = sys.stdin.read(1).lower()
			if choice and choice in choices:
				print(choice, end="" if choice == "\n" else "\n")
				return choice
	finally:
		termios.tcsetattr(fd, termios.TCSADRAIN, settings)


def run(command, *args, cwd=None, input=None, capture_stdout=False, capture_stderr=False, raise_on_nonzero=True):
	env = None
	if cwd:
		env = os.environ.copy()
		# Some programs read the PWD environment variable, and we're not calling a shell that automatically sets it, so set it here.
		env['PWD'] = os.path.abspath(cwd)
	stdout = subprocess.PIPE if capture_stdout else subprocess.DEVNULL
	# Merge streams when both are captured.
	if capture_stdout and capture_stderr:
		stderr = subprocess.STDOUT
	else:
		stderr = subprocess.PIPE if capture_stderr else subprocess.DEVNULL
	result = subprocess.run([command, *args], cwd=cwd, env=env, input=input, stdout=stdout, stderr=stderr, text=True)
	output = result.stdout if capture_stdout else result.stderr
	if raise_on_nonzero and result.returncode != 0:
		message = f"Command exited with status {result.returncode}:\n{shlex.join([command, *args])}"
		if cwd:
			message += f"\nWorking directory: {cwd!r}"
		if output:
			message += f"\n\n{output}"
		raise RuntimeError(message)
	return output


def run_retry(command, *args, cwd=None):
	while True:
		try:
			run(command, *args, cwd=cwd, capture_stderr=True)
			return
		except Exception as error:
			warn(error)
			if choose("[r]etry, [i]gnore: ", "ri") == "i":
				return
