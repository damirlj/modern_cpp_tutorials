import os
import xml.etree.ElementTree as ET
from datetime import datetime
from email.utils import parsedate_to_datetime

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
    channel = root.find("channel")
else:
    root = ET.Element("rss", version="2.0")
    channel = ET.SubElement(root, "channel")
    ET.SubElement(channel, "title").text = "Modern C++ Tutorials - Docs Updates"
    ET.SubElement(channel, "link").text = "https://github.com/damirlj/modern_cpp_tutorials"
    ET.SubElement(channel, "description").text = "New articles and updates in the docs/ folder"

# Map from GUID to <item> for quick lookup
existing_items = {}
for item in channel.findall("item"):
    guid = item.find("guid").text if item.find("guid") is not None else None
    if guid:
        existing_items[guid] = item

# Track updated list of items
new_items = []

# Generate items from current PDF list
for pdf in pdf_files:
    relative_path = os.path.relpath(pdf, docs_folder)
    commit_url = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/{relative_path}"

    # If this article already exists, preserve pubDate
    if commit_url in existing_items:
        old_item = existing_items[commit_url]
        pub_date = old_item.find("pubDate").text
        pub_datetime = parsedate_to_datetime(pub_date)
    else:
        # New item - use current time
        pub_datetime = datetime.utcnow()

    item = ET.Element("item")
    ET.SubElement(item, "title").text = relative_path
    ET.SubElement(item, "link").text = commit_url
    ET.SubElement(item, "guid").text = commit_url
    ET.SubElement(item, "pubDate").text = pub_datetime.strftime("%a, %d %b %Y %H:%M:%S GMT")

    new_items.append((pub_datetime, item))

# Sort items newest-first by pubDate
new_items.sort(key=lambda x: x[0], reverse=True)

# Clear old items and re-append in sorted order
for old_item in channel.findall("item"):
    channel.remove(old_item)
for _, item in new_items:
    channel.append(item)

# Save the updated RSS feed
tree = ET.ElementTree(root)
tree.write(rss_file, encoding="UTF-8", xml_declaration=True)

print(f"Generated RSS feed with {len(new_items)} articles.")
