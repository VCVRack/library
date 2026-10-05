import sys
import termios
import tty


def warn(message):
	print(f"\033[33m{message}\033[0m")


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
