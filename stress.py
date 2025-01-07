import requests
import concurrent.futures
import time
import random

# Define colors for console output
GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"


# Define multiple target URLs in a list
TARGET_URLS = [
    "http://localhost:8084",
    "http://localhost:8084/test",
    "http://localhost:8084/test/",
    "http://localhost:8084/hello",
    "http://localhost:8084/hello/",
   
]

METHODS = [
    "GET",
    "POST",
    "PUT",
    "DELETE",
]

TOTAL_REQUESTS = 100000
CONCURRENCY_LEVEL = 10000

def send_request(_):  
    url = random.choice(TARGET_URLS)
    method = random.choice(METHODS)
    try:
        response = requests.get(url) if method == "GET" else requests.post(url) if method == "POST" else requests.put(url) if method == "PUT" else requests.delete(url)
        print(f"Request to {url} returned status code {GREEN if response.status_code == 200 else RED}{response.status_code}{RESET}")
    except requests.exceptions.RequestException as e:
        print(f"Request to {url} failed: {e}")

def stress_test():
    start_time = time.time()

    with concurrent.futures.ThreadPoolExecutor(max_workers=CONCURRENCY_LEVEL) as executor:
        # Using range instead of TARGET_URL
        futures = [executor.submit(send_request, i) for i in range(TOTAL_REQUESTS)]
        concurrent.futures.wait(futures)

    end_time = time.time()
    print(f"\nStress test completed in {end_time - start_time:.2f} seconds.")

if __name__ == "__main__":
    stress_test()
