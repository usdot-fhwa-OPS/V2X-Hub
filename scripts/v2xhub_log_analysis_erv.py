import sys
import pandas as pd
import argparse

'''
Get file names
'''


def get_filenames():
    inputfile = ''
    outputfile = ''
    parser = argparse.ArgumentParser(prog="V2xHub Analysis for ERV BSM")
    parser.add_argument('--input', type=str, required=True)
    parser.add_argument('--output', type=str, required=True)
    args = parser.parse_args()
    print(f'Received log file: {args.input}; Result will be saved into file: {args.output}.xlsx')
    inputfile = args.input
    outputfile = args.output
    return (inputfile, outputfile)
'''
Read the input logs file and search the relevant logs. Process the logs and return a dictionary of the relevant information
'''


def process_input_log_file(inputfile, search_keyword):
    fields_dict = {}
    fields_dict["Time (UTC)"] = []
    try:
        file_stream = open(inputfile, 'r')
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
            txt_list = [x.strip() for x in  txt.strip().split(":") if x.strip() != 'INFO' if x.strip() != 'DEBUG' if x.strip() != 'ERROR' ]
            # Look for the specific metric by keyword
            if len(txt_list) > 1 and search_keyword.lower().strip() == txt_list[0].strip().lower():
                metadata_list = [x for x in line.strip().split(
                    ' - ')[0].split("[") if x != '']
                time = ''.join(metadata_list).split(']')[0].replace('"','').replace('log','').replace('{','').replace('{}','').replace('}','')
                metric_field_value = ''
                metric_field_title = ''            
                metric_field_title = txt_list[0].strip().replace("stream","").replace('\\n','').replace('"','').replace(',','')
                metric_field_value = txt_list[1].strip().replace("stream","").replace('\\n','').replace('"','').replace(',','').replace('\\u003e','>').replace('\\u003c','<').replace('\\','"')
                # The actual BSM XML content from the log is on the next line
                if metric_field_title.strip() in ['Incoming BSM is not from Emergency Response Vehicle (ERV)', 'Receive BSM'] :
                    metric_field_value = file_stream.__next__()
                    metric_field_value = metric_field_value.strip().replace("stream","").replace('\\n','').replace('"','').replace(',','').replace('\\u003e','>').replace('\\u003c','<').replace('\\','"')
                    metric_field_value_list = metric_field_value.split(':')
                    for value in metric_field_value_list:
                        if '<BasicSafetyMessage>' in value:
                            metric_field_value = value
                elif metric_field_title.strip() in ['Successfully forwarded message to cloud via curl', 'Forwarding message to cloud via curl']:
                    if '<BSMRequest>' not in metric_field_value:                   
                        continue
                if metric_field_title not in fields_dict.keys():
                    fields_dict[metric_field_title] = []                
                fields_dict[metric_field_title].append(metric_field_value)
                fields_dict["Time (UTC)"].append(time.replace('status:200',''))            
        file_stream.close()
    except FileNotFoundError:
        print(f'Log file not found {inputfile}')
        exit()
    return fields_dict


'''
main entrypoint to read carmacloud.log file and search based on the metric keyworkds. 
Once the relevant logs are found, it write the data into the specified excel output file.
'''


def main():    
    inputfile, outputfile = get_filenames()
    search_metric_keywords = {
                              'FER-4-5-8-9': 'Receive BSM',
                              'FER-10-11-1': 'Forwarding message to cloud via curl',
                              'FER-10-11-2': 'Successfully forwarded message to cloud via curl',
                              'FER-TBD-1': 'Received ERV BSM from cloud',
                              'FER-TBD-2': 'Received ERV BSM from cloud and broadcast ERV BSM delay(ms)',
                              'FER-TBD-3': 'Incoming BSM is not from Emergency Response Vehicle (ERV)',
                              'FER-TBD-4': 'Forward ERV BSM to cloud',
                              'FER-TBD-5': 'Received ERV BSM and forward ERV BSM to cloud delay (ms)',
                             }
    global_fields_dict = {}
    print(f'Processing log file [{inputfile}]...')
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
    main()
