import sys
import getopt
import pandas as pd
import xml.etree.ElementTree as ET

'''
Get file names
'''


def get_filenames(argv):
    inputfile = ''
    outputfile = ''
    try:
        if len(argv) < 4:
            print('python3 carmacloud_log_analysis.py -i <inputfile> -o <outputfile>')
        else:
            opts, args = getopt.getopt(args=argv, shortopts="i:s:o:")
            for opt, arg in opts:
                if opt == '-i':
                    inputfile = arg
                elif opt == "-o":
                    outputfile = arg
        return (inputfile,  outputfile)
    except:
        print('python3 carmacloud_log_analysis.py -i <inputfile> -o <outputfile>')
        sys.exit()


'''
Read the input logs file and search the relevant logs. Process the logs and return a dictionary of the relevant information
'''


def process_input_log_file(inputfile, search_keyword):
    file_stream = open(inputfile, 'r')
    fields_dict = {}
    fields_dict["Time (UTC)"] = []
    while True:
        line = file_stream.readline()
        # if line is empty, end of file is reached
        if not line:
            break
        if len(line.strip()) == 0:
            continue
        if len(line.strip().split(' - ')) < 2:
            continue

        txt = line.strip().split(' - ')[1]
        # Look for the specific metric by keyword
        if search_keyword.lower().strip() in txt.lower():
            metadata_list = [x for x in line.strip().split(
                ' - ')[0].split("[") if x != '']
            time = ''.join(metadata_list).split(']')[0].replace('"','').replace('log','').replace('{','').replace('{}','').replace(':','',1).replace('}','')
            fields_dict["Time (UTC)"].append(time)
            metric_field_value = ''
            metric_field_title = ''
            txt_list = [x.strip() for x in  txt.strip().split(":") if x.strip() != 'INFO' if x.strip() != 'DEBUG' if x.strip() != 'ERROR' ]
            if  "stdout" in txt_list[1]:
                metric_field_title = txt_list[0].strip().replace("stream","").replace('\\n','').replace('"','').replace(',','')
                metric_field_value = txt_list[0].strip().replace("stream","").replace('\\n','').replace('"','').replace(',','').replace('\\u003e','>').replace('\\u003c','<').replace('\\','"')
            else:
                metric_field_title = txt_list[0].strip().replace("stream","").replace('\\n','').replace('"','').replace(',','')
                metric_field_value = txt_list[1].strip().replace("stream","").replace('\\n','').replace('"','').replace(',','').replace('\\u003e','>').replace('\\u003c','<').replace('\\','"')
                try:
                    xml = ET.fromstring(metric_field_value)
                    metric_field_value = xml.find("id").text
                except:
                    pass
            if metric_field_title not in fields_dict.keys():
                fields_dict[metric_field_title] = []
            fields_dict[metric_field_title].append(metric_field_value)
    file_stream.close()
    return fields_dict


'''
main entrypoint to read carmacloud.log file and search based on the metric keyworkds. 
Once the relevant logs are found, it write the data into the specified excel output file.
'''


def main(argv):
    inputfile, outputfile = get_filenames(argv=argv)
    search_metric_keywords = {
                              'FER-5-6': 'Incoming BSM is not from Emergency Response Vehicle (ERV)',
                              'FER-9-10-11': 'Forward ERV BSM to cloud:',
                              'FER-12-1': 'Received ERV BSM and forward ERV BSM to cloud delay (ms)',
                              'FER-TBD-1': 'Received ERV BSM from cloud:',
                              'FER-TBD-2': 'Received ERV BSM from cloud and broadcast ERV BSM delay(ms)'
                             }
    global_fields_dict = {}
    for metric_keyword_key in search_metric_keywords.keys():
        if len(inputfile) > 0 and len(outputfile) > 0:
            fields_dict = process_input_log_file(inputfile=inputfile,
                                                 search_keyword=search_metric_keywords[metric_keyword_key])
            if len(fields_dict) > 0:
                global_fields_dict[metric_keyword_key] = fields_dict

    # Write dictionary into excel file
    if len(global_fields_dict) > 0:
        with pd.ExcelWriter(outputfile+".xlsx") as writer:
            for metric_keyword in global_fields_dict.keys():
                data_frame = pd.DataFrame(global_fields_dict[metric_keyword])
                data_frame.to_excel(writer, sheet_name=metric_keyword, index=False)
                print(f'Generated sheet for metric: {metric_keyword}')


if __name__ == '__main__':
    main(sys.argv[1:])
