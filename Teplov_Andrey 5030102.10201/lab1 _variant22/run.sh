# Minimum required g++ version
REQUIRED_GPP_VERSION=10

# Check if g++ is installed
if ! command -v g++ > /dev/null 2>&1; then
  echo "g++ is not installed. You can install it using:"
  echo "sudo apt update && sudo apt install g++"
  exit 1
fi

# Get the installed g++ version
GPP_VERSION=$(g++ -dumpversion | cut -d. -f1)

# Check if the installed g++ version is sufficient
# if [[ $GPP_VERSION -ge $REQUIRED_GPP_VERSION ]]; then
#   echo "g++ version $GPP_VERSION is installed."
# else
#   echo "g++ version $GPP_VERSION is installed, but version $REQUIRED_GPP_VERSION or higher is required."
#   echo "You can update g++ using:"
#   echo "sudo apt update && sudo apt install g++-$REQUIRED_GPP_VERSION"
#   exit 1
# fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Compile the program
g++ -std=c++20 "$SCRIPT_DIR/main.cpp" -o "$SCRIPT_DIR/main"

if [ $? -ne 0 ]; then
  echo "Compilation failed."
  exit 1
else

echo "Compilation successful. Running the program..."

# Run the executable
"$SCRIPT_DIR/main" "$SCRIPT_DIR/config.txt"

# Remove the executable after execution
rm "$SCRIPT_DIR/main"
