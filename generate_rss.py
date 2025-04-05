import os
import datetime

DOCS_DIR = "docs"
RSS_FILE = os.path.join(DOCS_DIR, "rss.xml")
BASE_URL = "https://damirlj.github.io/modern_cpp_tutorials/docs"

header = f"""<?xml version="1.0" encoding="UTF-8" ?>
<rss version="2.0">
<channel>
  <title>Modern C++ Tutorials</title>
  <link>{BASE_URL}/</link>
  <description>Latest articles in Modern C++ Tutorials</description>
  <lastBuildDate>{datetime.datetime.utcnow().strftime('%a, %d %b %Y %H:%M:%S +0000')}</lastBuildDate>
"""

# Collecting all PDF files in the docs folder, ordered by date
items = []
for file in sorted(os.listdir(DOCS_DIR), reverse=True):
    if file.endswith(".pdf"):
        title = file.replace("_", " ").replace(".pdf", "").title()
        link = f"{BASE_URL}/{file}"
        pub_date = datetime.datetime.utcfromtimestamp(os.path.getmtime(os.path.join(DOCS_DIR, file))).strftime('%a, %d %b %Y %H:%M:%S +0000')
        items.append(f"""
  <item>
    <title>{title}</title>
    <link>{link}</link>
    <guid>{link}</guid>
    <pubDate>{pub_date}</pubDate>
  </item>""")

footer = """
</channel>
</rss>
"""

# If rss.xml exists, update it; else create a new one
if os.path.exists(RSS_FILE):
    with open(RSS_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    # Remove the old <channel> content before adding new items
    new_content = content.split("</channel>")[0] + "</channel>" + footer
    with open(RSS_FILE, "w", encoding="utf-8") as f:
        f.write(new_content)
else:
    # If RSS file doesn't exist, create it from scratch
    with open(RSS_FILE, "w", encoding="utf-8") as f:
        f.write(header + "\n".join(items) + footer)

print("✅ RSS feed updated for PDFs!")
