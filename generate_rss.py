import os
import xml.etree.ElementTree as ET
from datetime import datetime

# Path to your docs folder
docs_folder = 'docs'
rss_file = 'docs/rss.xml'

# Get a list of all PDF files in docs and its subdirectories
pdf_files = []
for root, dirs, files in os.walk(docs_folder):
    for file in files:
        if file.endswith(".pdf"):
            pdf_files.append(os.path.join(root, file))

print(f"Found {len(pdf_files)} PDFs in the 'docs' folder.")

# Create or load RSS XML
if os.path.exists(rss_file):
    tree = ET.parse(rss_file)
    root = tree.getroot()
else:
    root = ET.Element("rss", version="2.0")
    channel = ET.SubElement(root, "channel")
    ET.SubElement(channel, "title").text = "Modern C++ Tutorials - Docs Updates"
    ET.SubElement(channel, "link").text = "https://github.com/damirlj/modern_cpp_tutorials"
    ET.SubElement(channel, "description").text = "New articles and updates in the docs/ folder"

# Get current date for publishing
current_date = datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")

# Iterate through each PDF file found
for pdf in pdf_files:
    # Convert the full file path to a relative URL for GitHub
    relative_path = os.path.relpath(pdf, docs_folder)
    commit_url = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/{relative_path}"

    # Create a new RSS item for each PDF
    item = ET.Element("item")
    ET.SubElement(item, "title").text = relative_path  # Use relative path as title
    ET.SubElement(item, "link").text = commit_url
    ET.SubElement(item, "guid").text = commit_url
    ET.SubElement(item, "pubDate").text = current_date

    # Append the item to the channel
    channel = root.find("channel")
    channel.append(item)

# Save the updated RSS feed
tree = ET.ElementTree(root)
tree.write(rss_file, encoding="UTF-8", xml_declaration=True)

print(f"Generated RSS feed with {len(pdf_files)} articles.")
