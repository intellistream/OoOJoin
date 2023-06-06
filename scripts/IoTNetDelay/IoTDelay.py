import subprocess
import csv

# Run the ping command and capture the output
rounds=1000


# Parse the output and extract the "time=" information
time_values = []
for k in range(rounds):
    ping_process = subprocess.Popen(['ping','-w','200', '-c', '5', '10.12.13.213'], stdout=subprocess.PIPE)
    ping_output, _ = ping_process.communicate()
    for line in ping_output.decode().split('\n'):
        if 'time=' in line:
            time_start = line.index('time=') + len('time=')
            time_end = line.index(' ', time_start)
            time_value = line[time_start:time_end]
            time_values.append(time_value)
            print(float(time_value))

# Write the time values to a CSV file
with open('ping_times.csv', 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['Latency'])
    writer.writerows([[value] for value in time_values])
