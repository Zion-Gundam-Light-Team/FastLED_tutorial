#!/bin/bash
# Script to clean unnecessary files from SPIFFS data folder

echo "Cleaning SPIFFS data folder..."

# Remove macOS system files
rm -f data/html/.DS_Store
rm -f data/.DS_Store

# Optional: Remove logo to save 25KB (uncomment if needed)
# rm -f data/html/logo.png
# echo "Removed logo.png (25KB saved)"

# Optional: Remove master.html if you don't update master via web (uncomment if needed)
# rm -f data/html/master.html
# echo "Removed master.html (3.5KB saved)"

echo "SPIFFS cleanup complete!"
echo ""
echo "Current SPIFFS data size:"
du -sh data/
echo ""
echo "Files in data/html:"
ls -lah data/html/