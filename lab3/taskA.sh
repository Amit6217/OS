    #!/bin/bash

if [ $# -eq 0 ]; then
    echo "Please provide a filename."
    exit 1
fi

FILE="$1"

if [ ! -e "$FILE" ]; then
    echo "File '$FILE' does not exist."
    exit 1
fi

echo "Information about: $FILE"
echo "--------------------------------"
echo "Size: $(stat -c%s "$FILE") bytes"
echo "Permissions: $(stat -c%A "$FILE")"
echo "Owner: $(stat -c%U "$FILE")"
echo "Modification Time: $(stat -c%y "$FILE")"
