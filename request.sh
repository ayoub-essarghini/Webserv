# Define the boundary string for multipart/form-data
BOUNDARY="----WebKitFormBoundary7MA4YWxkTrZu0gW"

# Get the contents of the file
FILE_CONTENT=$(cat file.txt)

# Calculate the Content-Length of the body (it's the sum of all parts, including the file data)
CONTENT_LENGTH=$(echo -n "$FILE_CONTENT" | wc -c)

# Send the POST request using nc
(
echo -e "POST /test/test2/ HTTP/1.1"
echo -e "Host: ayoub"
echo -e "Content-Type: multipart/form-data; boundary=$BOUNDARY"
echo -e "Content-Length: $CONTENT_LENGTH"
echo -e ""  # Empty line to separate headers from body

# Body of the request with file content
echo -e "--$BOUNDARY"
echo -e "Content-Disposition: form-data; name=\"file\"; filename=\"file.txt\""
echo -e "Content-Type: text/plain"
echo -e ""
echo -e "$FILE_CONTENT"
echo -e ""
echo -e "--$BOUNDARY--"
) | nc -v 0 8084
