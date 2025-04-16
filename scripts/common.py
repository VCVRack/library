import subprocess


def system(cmd):
	print(cmd)
	result = subprocess.run(cmd, shell=True)
	if result.returncode != 0:
		raise Exception(f"Command failed with error {result.returncode}: {cmd}")

def system_retry(cmd):
	while True:
		try:
			system(cmd)
			break
		except Exception as error:
			print(error)
			print("Enter to retry")
			input()


def run(cmd):
	print(cmd)
	result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE)
	if result.returncode != 0:
		raise Exception(f"Command failed with error {result.returncode}: {cmd}")
	return result.stdout.decode('utf-8')


def find(list, f):
	return next((x for x in list if f(x)), None)
