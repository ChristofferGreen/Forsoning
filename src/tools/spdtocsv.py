import csv

def line_message(line):
    cs = line.rsplit(']', 1)
    return cs[1]

def line_thread(line):
    cs = line.rsplit(']', 1)
    cols = cs[0].replace("]", "").split("[")
    if(len(cols)>=5):
        return cols[4]
    return ""

def read_columns(lines):
    cols = set()
    for line in lines:
        cols.add(line_thread(line))
    return cols

def get_column(columns, column):
    return list(columns).index(column)

def create_row(column, nbr_columns, message):
    row = []
    for i in range(0, nbr_columns):
        if(i==column):
            row.append(message)
        else:
            row.append('')
    return row

with open('logs/basic-log.txt') as f, open('logs/basic-log.csv', 'w', newline='') as csvfile:
    csvwriter = csv.writer(csvfile)
    lines = f.readlines()
    columns = read_columns(lines)
    nbr_columns = len(columns)
    for line in lines:
        csvwriter.writerow(create_row(get_column(columns, line_thread(line)), nbr_columns, line_message(line)))

