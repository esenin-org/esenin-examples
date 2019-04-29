import csv

with open('lenta-ru-news.csv', newline='') as csvfile, open('lenta-ru-news-texts.txt', 'w') as write_file:
    reader = csv.DictReader(csvfile)
    for row in reader:
         write_file.write(row['text'])
         write_file.write("\n")