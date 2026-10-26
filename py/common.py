import os
import re
import shlex
import subprocess
import sys
import termios
import tty
from urllib.parse import urldefrag, urlsplit

import requests

from . import config


URL_CACHE = set()


def format_items(items):
	"""Formats an iterable of strings as an English list."""
	items = list(items)
	if len(items) <= 2:
		return " and ".join(items)
	return ", ".join(items[:-1]) + ", and " + items[-1]


def is_valid_slug(slug):
	return bool(re.fullmatch(r"[A-Za-z0-9](?:[A-Za-z0-9_-]*[A-Za-z0-9])?", slug))


def is_valid_url(url):
	if any(character.isspace() for character in url):
		return False
	try:
		parts = urlsplit(url)
		if parts.scheme not in ("http", "https"):
			return False
		if not parts.hostname:
			return False
	except ValueError:
		return False
	return True


def review_url(url):
	if not is_valid_url(url):
		raise ValueError(f"URL `{url}` is not a valid HTTP or HTTPS URL.")
	request_url = urldefrag(url).url
	if request_url in URL_CACHE:
		return
	headers = {"User-Agent": config.HTTP_USER_AGENT}
	with requests.get(request_url, headers=headers, timeout=config.HTTP_TIMEOUT, stream=True) as response:
		response.raise_for_status()
	URL_CACHE.add(request_url)


def version_key(version):
	parts = []
	for part in version.split('.'):
		try:
			parts.append((1, int(part)))
		except ValueError:
			parts.append((0, part))
	return parts


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
