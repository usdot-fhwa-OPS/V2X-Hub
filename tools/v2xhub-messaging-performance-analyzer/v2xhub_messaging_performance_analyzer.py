"""
V2X Hub Messaging Performance Analyzer
This script analyzes messaging performance between a V2X Hub and RSU
by reading log files, calculating message drop, latency, and throughput,
and generating plots for visualization.
"""

import os
import re
import argparse
import logging
import pandas as pd
import matplotlib.pyplot as plt


# Define main function which reads in input_dir using argparse

def read_log_files(input_dir):
    """
    Reads input log files from RSU/V2X Hub in input_dir and returns
    two dataframes: v2xhub_tx_logs and rsu_tx_logs.
    
    :param input_dir: Description
    """
    rsu_tx_logs = None
    v2xhub_tx_logs = None

    # Read log files from input directory
    for filename in os.listdir(input_dir):
        if filename.startswith('j2735') and "Tx" in filename and filename.endswith('.log'):
            logging.debug('Reading V2X Hub log file: %s', filename)
            v2xhub_tx_logs = read_log_to_dataframe(os.path.join(input_dir, filename))
        elif  'tx' in filename and filename.endswith('.log'):
            logging.debug('Reading RSU log file: %s', filename)
            rsu_tx_logs = read_log_to_dataframe(os.path.join(input_dir, filename))
    return v2xhub_tx_logs, rsu_tx_logs

def read_log_to_dataframe(log_file):
    """
    Reads *.log file as csv with separator " : "  and returns dataframe

    :param log_file: Description
    """
    data =  pd.read_csv(
        log_file, sep=" : ",
        header=None,
        engine='python',
        names= ['Timestamp', 'Raw JSON String']
    )
    # Regex to replace any boolean, null, or numeric values surrounded by quotes with unquoted
    # values
    data['Cleaned JSON String'] = data['Raw JSON String'].apply(
        lambda x: re.sub(r'\"(-?[0-9]+(\\.[0-9]+)?|null|true|false)\"', r'\1', x))
    # Add this debug code before the DataFrame creation
    data.to_csv(
        "data/" + os.path.basename(log_file) + ".csv",
        index=False,
        columns=['Timestamp', 'Cleaned JSON String']
    )
    return data

def calculate_messsage_performance(tx_log, rx_log):
    """
    Docstring for calculate_messsage_performance by
    looping through tx_log, finding matching message in
    rx_log, and calculating drop, latency.
    Returns dataframe for message drop and latency

    :param v2xhub_tx_logs: Description
    :param rsu_tx_logs: Description
    """
    message_drop =[[]]
    latency = [[]]
    for index, tx_row in tx_log.iterrows():
        tx_timestamp = tx_row['Timestamp']
        tx_message = tx_row['Cleaned JSON String']
        # Find matching message in rx_log
        rx_row = rx_log[rx_log['Cleaned JSON String'] == tx_message]
        if not rx_row.empty:
            rx_timestamp = rx_row.iloc[0]['Timestamp']
            latency.append( (tx_timestamp, rx_timestamp - tx_timestamp))
            # Remove the matched row to avoid duplicate matches
            rx_log = rx_log.drop(rx_row.index[0])
            if rx_timestamp - tx_timestamp > 100:
                logging.debug(
                    'High latency deteected. Potential mismatch at index Tx %d and Rx %d: Tx Timestamp %d, Rx Timestamp %d, Latency %d ms', 
                    index, rx_row.index[0], tx_timestamp, rx_timestamp, rx_timestamp - tx_timestamp)
        else:
            message_drop.append( (tx_timestamp, tx_message))
    latency_df = pd.DataFrame(latency, columns=['Tx Timestamp', 'Latency (ms)'])
    if len(message_drop) > 1:
        message_drop_df = pd.DataFrame(message_drop, columns=['Tx Timestamp', 'Message'])
    else:
        message_drop_df = pd.DataFrame(columns=['Tx Timestamp', 'Message'])
    return message_drop_df, latency_df

def plot_latency(latency_df, output_dir):
    """
    Plots latency over time and saves to output_dir

    :param latency_df: Description
    :param output_dir: Description
    """
    plt.set_loglevel('warning')
    plt.figure(figsize=(10, 6))
    # Convert Tx Timestamp to datetime for better x-axis representation
    latency_df['Tx DateTime'] = pd.to_datetime(latency_df['Tx Timestamp'], unit='ms')
    plt.scatter(latency_df['Tx DateTime'], latency_df['Latency (ms)'], marker='o')
    plt.title('Message Latency Over Time')
    plt.xlabel('Time')
    plt.ylabel('Latency (ms)')
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'message_latency.png'))
    plt.close()

def plot_message_drop(message_drop_df, output_dir):
    """
    Plots message drop over time and saves to output_dir

    :param message_drop_df: Description
    :param output_dir: Description
    """
    plt.set_loglevel('warning')
    plt.figure(figsize=(10, 6))
    # Convert Tx Timestamp to datetime for better x-axis representation
    message_drop_df['Tx DateTime'] = pd.to_datetime(message_drop_df['Tx Timestamp'], unit='ms')
    plt.scatter(message_drop_df['Tx DateTime'], [1]*len(message_drop_df), marker='x', color='red')
    plt.title('Message Drops Over Time')
    plt.xlabel('Time')
    plt.yticks([])
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'message_drops.png'))
    plt.close()

def plot_throughput(tx_log, rx_log, output_dir):
    """
    Plot V2X Hub and RSU throughput over time and saves to output_dir
    :param tx_log: Description
    :param rx_log: Description
    :param output_dir: Description
    """
    plt.set_loglevel('warning')
    plt.figure(figsize=(10, 6))
    # Convert Timestamp to datetime for better x-axis representation
    tx_log['DateTime'] = pd.to_datetime(tx_log['Timestamp'], unit='ms')
    rx_log['DateTime'] = pd.to_datetime(rx_log['Timestamp'], unit='ms')
    # Resample to 1 second intervals and count messages
    tx_throughput = tx_log.resample('1s', on='DateTime').size()
    rx_throughput = rx_log.resample('1s', on='DateTime').size()
    plt.plot(tx_throughput.index, tx_throughput.values, label='V2X Hub Throughput', color='blue')
    plt.plot(rx_throughput.index, rx_throughput.values, label='RSU Throughput', color='green')
    plt.title('Throughput Over Time')
    plt.xlabel('Time')
    plt.ylabel('Messages per Second')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'throughput.png'))
    plt.close()
def main():
    """
    Reads files from input_directory, calculates messaging performance metricss
    and generates plots saved to ./plots directory.
    """

    parser = argparse.ArgumentParser(
        description='Analyze V2X messaging performance from log files.'
    )
    parser.add_argument(
        '--input_dir', 
        type=str,
        help='Directory containing RSU and V2X Hub log files'
    )
    parser.add_argument(
        '--debug', 
        action='store_true', 
        help='Enable debug logging'
    )
    args = parser.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    input_dir = args.input_dir
    # Check input directory exists
    if not os.path.isdir(input_dir):
        print(f"Input directory {input_dir} does not exist.")
        return
    v2xhub_tx_logs, rsu_tx_logs = read_log_files(input_dir)
    # Debug log first 5 rows of each dataframe
    logging.debug('V2X Hub TX Logs:\n%s', v2xhub_tx_logs.head())
    logging.debug('RSU TX Logs:\n%s', rsu_tx_logs.head())
    message_drop_df, latency_df = calculate_messsage_performance(v2xhub_tx_logs, rsu_tx_logs)
    logging.debug('Message Latency for first 5 messages:\n%s', latency_df.head())
    plot_latency(latency_df, './plots')
    plot_throughput(v2xhub_tx_logs, rsu_tx_logs, './plots')
    if not message_drop_df.empty:
        logging.info('Message Drop detected for %d messages.', len(message_drop_df))
        plot_message_drop(message_drop_df, './plots')

    
    # Calculate message drop, latency, and throughput

if __name__ == "__main__":
    main()