import os
import xml.etree.ElementTree as ET
from datetime import datetime, timezone
from email.utils import parsedate_to_datetime

# Path to your docs folder and RSS file
docs_folder = 'docs'
rss_file = os.path.join(docs_folder, 'rss.xml')

# Collect all PDF files in docs/
pdf_files = []
for root, dirs, files in os.walk(docs_folder):
    for file in files:
        if file.endswith(".pdf"):
            pdf_files.append(os.path.join(root, file))

print(f"Found {len(pdf_files)} PDFs in the 'docs' folder.")

# Load existing RSS or create a new one
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

# Map existing GUIDs to pubDate
existing_items = {
    item.find("guid").text: item.find("pubDate").text
    for item in channel.findall("item") if item.find("guid") is not None
}

# Prepare new items
new_items = []
current_date = datetime.now(timezone.utc).strftime("%a, %d %b %Y %H:%M:%S GMT")
changes_made = False

for pdf in sorted(pdf_files):
    relative_path = os.path.relpath(pdf, docs_folder)
    link = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/{relative_path}"

    # Check if the item already exists
    item = ET.Element("item")
    ET.SubElement(item, "title").text = relative_path
    ET.SubElement(item, "link").text = link
    ET.SubElement(item, "guid").text = link

    # Preserve existing pubDate or assign new one
    if link in existing_items:
        pub_date = existing_items[link]  # Preserve timestamp of existing item
        # Remove the old version if it's updated (duplicate)
        for old_item in channel.findall("item"):
            if old_item.find("guid").text == link:
                channel.remove(old_item)
                changes_made = True
                break
    else:
        pub_date = current_date  # New items get the current date

    ET.SubElement(item, "pubDate").text = current_date if link not in existing_items else pub_date

    # If the item is new, add it to the list of new items
    if link not in existing_items:
        new_items.append((parsedate_to_datetime(pub_date), item))
        changes_made = True
    else:
        # For updated items, we already removed the old item and added the new one
        new_items.append((parsedate_to_datetime(pub_date), item))

# If no changes were made (no new items and no updates), exit early
if not changes_made:
    print("No new or updated items found. Exiting without writing.")
    exit(0)

# Sort with newest items at the top
new_items.sort(key=lambda tup: tup[0], reverse=True)

# Clear old items and append new ones (sorted)
for old_item in channel.findall("item"):
    channel.remove(old_item)

for _, item in new_items:
    channel.append(item)

# Save the updated RSS feed
tree = ET.ElementTree(root)
tree.write(rss_file, encoding="UTF-8", xml_declaration=True)
print(f"Generated RSS feed with {len(new_items)} articles.")
