import os
import xml.etree.ElementTree as ET
from datetime import datetime, timezone
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

# Create a dictionary of existing items by GUID (commit URL)
existing_items = {item.find("guid").text: item for item in channel.findall("item")}

# Get current UTC date with tzinfo
current_date = datetime.now(timezone.utc).strftime("%a, %d %b %Y %H:%M:%S GMT")

# Add or update items
for pdf in pdf_files:
    relative_path = os.path.relpath(pdf, docs_folder)
    commit_url = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/{relative_path}"

    # If item exists, update pubDate; otherwise, create new
    if commit_url in existing_items:
        existing_items[commit_url].find("pubDate").text = current_date
    else:
        item = ET.Element("item")
        ET.SubElement(item, "title").text = relative_path
        ET.SubElement(item, "link").text = commit_url
        ET.SubElement(item, "guid").text = commit_url
        ET.SubElement(item, "pubDate").text = current_date
        channel.append(item)

# Sort all items by pubDate descending
def get_pub_date(item):
    pub_date_text = item.find("pubDate").text
    dt = parsedate_to_datetime(pub_date_text)
    return dt if dt.tzinfo else dt.replace(tzinfo=timezone.utc)

items = channel.findall("item")
items.sort(key=get_pub_date, reverse=True)

# Clear old items and re-append in sorted order
for item in channel.findall("item"):
    channel.remove(item)
for item in items:
    channel.append(item)

# Save the updated RSS feed
tree = ET.ElementTree(root)
tree.write(rss_file, encoding="UTF-8", xml_declaration=True)

print(f"Generated RSS feed with {len(items)} article(s).")
