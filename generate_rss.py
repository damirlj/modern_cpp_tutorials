import os
import xml.etree.ElementTree as ET
from datetime import datetime

# Define the RSS file and folder containing articles
docs_folder = "docs"
rss_file = os.path.join(docs_folder, "rss.xml")  # Adjust path to rss.xml
current_date = datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")

def create_empty_rss_file(file_path):
    """
    Create an empty RSS file with the basic structure if it doesn't exist.
    :param file_path: Path to the RSS file.
    """
    root = ET.Element("rss", version="2.0")
    channel = ET.SubElement(root, "channel")
    ET.SubElement(channel, "title").text = "Modern C++ Tutorials"
    ET.SubElement(channel, "link").text = "https://github.com/damirlj/modern_cpp_tutorials"
    ET.SubElement(channel, "description").text = "RSS feed for Modern C++ Tutorials"
    tree = ET.ElementTree(root)
    tree.write(file_path, encoding="utf-8", xml_declaration=True)
    print(f"Created new RSS file: {file_path}")

def is_file_updated(file_path, existing_pub_date):
    """
    Check if the file has been updated based on its modification timestamp.
    :param file_path: Path to the file.
    :param existing_pub_date: Existing publication date in RSS feed.
    :return: True if the file has been updated, False otherwise.
    """
    file_mod_time = os.path.getmtime(file_path)
    existing_time = datetime.strptime(existing_pub_date, "%a, %d %b %Y %H:%M:%S GMT").timestamp()
    return file_mod_time > existing_time

# Check if the RSS file exists, and create it if necessary
if not os.path.exists(rss_file):
    create_empty_rss_file(rss_file)

# Load the existing RSS feed
tree = ET.parse(rss_file)
root = tree.getroot()
channel = root.find("channel")
existing_items = {
    item.find("link").text: item.find("pubDate").text
    for item in channel.findall("item")
}

# Get the list of PDF files in the docs folder
pdf_files = [os.path.join(docs_folder, f) for f in os.listdir(docs_folder) if f.endswith(".pdf")]

# Track changes
changes_made = False

# Process each PDF file
for pdf in sorted(pdf_files):
    relative_path = os.path.relpath(pdf, docs_folder)
    link = f"https://github.com/damirlj/modern_cpp_tutorials/blob/main/{relative_path}"

    # Check if the item already exists
    if link in existing_items:
        pub_date = existing_items[link]

        # Use the separate function to check if the file is updated
        if is_file_updated(pdf, pub_date):
            pub_date = current_date  # Update pubDate for modified items
            changes_made = True
    else:
        # Assign the current date for new items
        pub_date = current_date
        changes_made = True

    # Create or update the item
    item = ET.Element("item")
    ET.SubElement(item, "title").text = relative_path
    ET.SubElement(item, "link").text = link
    ET.SubElement(item, "guid").text = link
    ET.SubElement(item, "pubDate").text = pub_date

    # Add the item to the channel
    channel.append(item)

# Sort items in descending order by pubDate (newest first)
items = channel.findall("item")
sorted_items = sorted(items, key=lambda x: datetime.strptime(x.find("pubDate").text, "%a, %d %b %Y %H:%M:%S GMT"), reverse=True)

# Clear existing items and add the sorted items
channel.clear()
for item in sorted_items:
    channel.append(item)

# Save the updated RSS feed
if changes_made:
    tree.write(rss_file, encoding="utf-8", xml_declaration=True)
    print("RSS feed updated.")
else:
    print("No changes made to the RSS feed.")
