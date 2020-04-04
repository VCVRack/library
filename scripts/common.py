import subprocess


def system(cmd):
	print(cmd)
	result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE)
	if result.returncode != 0:
		raise Exception(f"Command failed with error {result.returncode}: {cmd}")
	return result.stdout.decode('utf-8')
