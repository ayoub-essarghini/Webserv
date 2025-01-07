import requests
import random
import string

# URL to which the POST request will be sent
url = "http://localhost:8084/"

# Size of the "large file" to simulate (e.g., 50 MB)
large_file_size = 50 * 1024 * 1024  # 50 MB
chunk_size = 1024 * 1024  # 1 MB per chunk

# Function to generate random data of a given size
def generate_large_data(size):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=size)).encode('utf-8')

# Function to send data in chunks
def send_large_data(url, large_data, chunk_size):
    # Create a generator that yields data in chunks
    def data_generator():
        for i in range(0, len(large_data), chunk_size):
            yield large_data[i:i+chunk_size]

    # Send POST request with data in chunks
    response = requests.post(url, data=data_generator())

    # Check response
    if response.status_code == 200:
        print("Data sent successfully.")
    else:
        print(f"Failed to send data. Status code: {response.status_code}")

# Generate large data (50 MB) in memory
large_data = generate_large_data(large_file_size)

# Send the simulated large data in chunks
send_large_data(url, large_data, chunk_size)
