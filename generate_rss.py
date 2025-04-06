import os
import xml.etree.ElementTree as ET
from datetime import datetime
from email.utils import format_datetime, parsedate_to_datetime

# Path to your docs folder
docs_folder = 'docs'
rss_file = os.path.join(docs_folder, 'rss.xml')

# Get a list of all PDF files in docs and subdirectories
pdf_files = []
for root_dir, dirs, files in os.walk(docs_folder):
    for file in files:
        if file.endswith(".pdf"):
            pdf_files.append(os.path.join(root_dir, file))

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

# Create a map of current items to remove duplicates
existing_items = {item.find("guid").text: item for item in channel.findall("item") if item.find("guid") is not None}

# Get current date in RFC 2822 format (for RSS)
current_date = format_datetime(datetime.utcnow())

# Add or update RSS items
for pdf in pdf_files:
    relative_path = os.path.relpath(pdf, docs_folder)
    commit_url = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/{relative_path}"

    # Remove old item if it exists
    if commit_url in existing_items:
        channel.remove(existing_items[commit_url])

    # Create and add the new item
    item = ET.Element("item")
    ET.SubElement(item, "title").text = relative_path
    ET.SubElement(item, "link").text = commit_url
    ET.SubElement(item, "guid").text = commit_url
    ET.SubElement(item, "pubDate").text = current_date
    channel.append(item)

# Sort items by pubDate descending
items = channel.findall("item")

# Parse pubDate strings to datetime objects for sorting
def get_pub_date(item):
    pub_date = item.find("pubDate").text
    return parsedate_to_datetime(pub_date)

items.sort(key=get_pub_date, reverse=True)

# Optional: Keep only the latest N entries (e.g., 20)
#MAX_ENTRIES = 20
#for item in channel.findall("item"):
#    channel.remove(item)
#for item in items[:MAX_ENTRIES]:
#    channel.append(item)

# Save updated RSS feed
tree = ET.ElementTree(root)
tree.write(rss_file, encoding="UTF-8", xml_declaration=True)

print(f"Generated RSS feed with {min(len(items), MAX_ENTRIES)} articles.")
