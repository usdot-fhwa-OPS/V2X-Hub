#!/usr/bin/env python3
"""
V2X Hub Messaging Performance Analyzer
This script analyzes messaging performance between a message source and a message destination
by reading log files, calculating message drop, latency, and throughput,
and generating plots for visualization.
"""

import os
import re
import sys
import argparse
import logging
import json

from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QApplication, QFileDialog

import pandas as pd
import matplotlib.pyplot as plt

import argparse
from pathlib import Path


def select_log_files(input_file_one=None,input_file_two=None):
    """
    Opens file dialogs for the user to select the originating source file and the destination file. Returns the selected file paths.
    """

    # Updated hardcoded input file paths (bypasses UI)
    if input_file_one and input_file_two:
        # If files were provided via command line, return them immediately
        return input_file_one, input_file_two
    
    app = QApplication.instance() or QApplication(sys.argv)

    file_filter = 'Log files (*.log);;All files (*)'

    source_file, _ = QFileDialog.getOpenFileName(
        None,
        'Select Source Log File',
        '',
        file_filter
    )
    if not source_file:
        logging.error('No source log file selected.')
        return None, None

    destination_file, _ = QFileDialog.getOpenFileName(
        None,
        'Select Destination Log File',
        '',
        file_filter
    )
    if not destination_file:
        logging.error('No destination log file selected.')
        return None, None

    # Briefly run the event loop so the dialog window fully closes before returning control to the caller for processing.
    QTimer.singleShot(1, app.quit)
    app.exec()

    return source_file, destination_file

def read_log_to_dataframe(log_file,data_dir):
    """
    Reads *.log file, joining lines that are continuations of multiline
    JSON entries, and returns dataframe with Timestamp and Raw JSON String.

    :param log_file: Description
    """
    entry_pattern = re.compile(r'^(\d+)\s+:\s+(.*)')
    rows = []
    with open(log_file, 'r') as f:
        current_ts = None
        current_json = None
        for line in f:
            line = line.rstrip('\n')
            match = entry_pattern.match(line)
            if match:
                # Save previous entry if exists
                if current_ts is not None:
                    rows.append((int(current_ts), current_json.strip()))
                current_ts = match.group(1)
                current_json = match.group(2)
            else:
                # Continuation of previous entry's JSON
                if current_json is not None:
                    current_json += line
        # Save last entry
        if current_ts is not None:
            rows.append((int(current_ts), current_json.strip()))
    data = pd.DataFrame(rows, columns=['Timestamp', 'Raw JSON String'])

    # Regex to replace any boolean, null, or numeric values surrounded by quotes with unquoted
    # values
    data['Cleaned JSON String'] = data['Raw JSON String'].apply(
        lambda x: re.sub(r'\"(-?[0-9]+(\\.[0-9]+)?|null|true|false)\"', r'\1', x))
    # Create Column for message id
    # Message Id to name map
    message_id_to_name = {
        "18": "MAP",
        "19": "SPAT",
        "31": "TIM",
        # Add more mappings as needed
    }
    # Function to extract messageId and convert to name, with error handling for JSON decode errors
    def convert_message_id(x):
        try:
            msg_id = str(json.loads(x).get('messageId'))
            return message_id_to_name.get(msg_id, f"Unknown({msg_id})")
        except json.decoder.JSONDecodeError as e:
            logging.error(f"Problem with JSON payload {x}: {e}")
            return "Unknown"

    data['Message Type'] = data['Raw JSON String'].apply(
        lambda x: 
            convert_message_id(x)
        )
    # Add this debug code before the DataFrame creation
    csv_path = data_dir / (os.path.basename(log_file) + ".csv")
    data.to_csv(
        csv_path,
        index=False,
        columns=['Timestamp', 'Message Type', 'Cleaned JSON String']
    )
    return data

def calculate_messsage_performance(tx_log, rx_log):
    """
    Docstring for calculate_messsage_performance by
    looping through tx_log, finding matching message in
    rx_log, and calculating drop, latency.
    Returns dataframe for message drop and latency

    :param tx_logs: Description
    :param rx_logs: Description
    """
    message_drop =[[]]
    latency = [[]]
    for index, tx_row in tx_log.iterrows():
        tx_timestamp = tx_row['Timestamp']
        tx_message = tx_row['Cleaned JSON String']
        tx_message_id = tx_row['Message Type']

        # Find matching message in rx_log
        # make case insensitive to avoid capitalization differences in OCTET STRING encoding between ASN1C and pycrate
        rx_row = rx_log[rx_log['Cleaned JSON String'].str.lower() == tx_message.lower()]
        if not rx_row.empty:
            rx_timestamp = rx_row.iloc[0]['Timestamp']

            if rx_timestamp - tx_timestamp > 200 :
                message_drop.append( (tx_timestamp, tx_message_id, tx_message))
            else:
                latency.append( (tx_timestamp, tx_message_id, rx_timestamp - tx_timestamp))
                # Remove the matched row to avoid duplicate matches
                rx_log = rx_log.drop(rx_row.index[0])
        else:
            message_drop.append( (tx_timestamp, tx_message_id, tx_message))
    if len(latency) > 1 :
        latency_df = pd.DataFrame(latency, columns=['Tx Timestamp', 'Message Type', 'Latency (ms)'])
    else:
        latency_df = pd.DataFrame( columns=['Tx Timestamp', 'Message Type', 'Latency (ms)']) 
    if len(message_drop) > 1:
        message_drop_df = pd.DataFrame(message_drop, columns=['Tx Timestamp', 'Message Type', 'Message'])
    else:
        message_drop_df = pd.DataFrame(columns=['Tx Timestamp', 'Message Type', 'Message'])
    return message_drop_df, latency_df

def plot_latency(latency_df, output_dir):
    """
    Plots latency for each message id over time and saves to output_dir

    :param latency_df: Description
    :param output_dir: Description
    """
    plt.set_loglevel('warning')
    fig, ax = plt.subplots(figsize=(10, 6))
    # Convert Tx Timestamp to datetime for better x-axis representation
    latency_df['Tx DateTime'] = pd.to_datetime(latency_df['Tx Timestamp'], unit='ms')
    markers = ['o', '^', 's', 'D', '*']
    marker_index = 0
    for message_id in latency_df['Message Type'].unique():
        subset = latency_df[latency_df['Message Type'] == message_id]
        ax.scatter(subset['Tx DateTime'], subset['Latency (ms)'], label=f'Message Type {message_id}', marker=markers[marker_index])
        marker_index = (marker_index + 1) % len(markers)
    plt.legend()
    plt.title('Message  Latency Over Time')
    plt.xlabel('Time')
    plt.ylabel('Latency (ms)')
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'message_latency.png'))
    plt.close()

def plot_message_drop(message_drop_df, output_dir):
    """
    Plots message drop for each message id over time and saves to output_dir

    :param message_drop_df: Description
    :param output_dir: Description
    """
    plt.set_loglevel('warning')
    fig, ax = plt.subplots(figsize=(10, 6))
    # Convert Tx Timestamp to datetime for better x-axis representation
    message_drop_df['Tx DateTime'] = pd.to_datetime(message_drop_df['Tx Timestamp'], unit='ms')
    markers = ['o', '^', 's', 'D', '*'] 
    marker_index = 0
    for message_id in message_drop_df['Message Type'].unique():
        subset = message_drop_df[message_drop_df['Message Type'] == message_id]
        ax.scatter(subset['Tx DateTime'], [1]*len(subset), label=f'Message Type {message_id}', marker=markers[marker_index])
        marker_index = (marker_index + 1) % len(markers)
    plt.legend()
    plt.title('Message Drops Over Time')
    plt.xlabel('Time')
    plt.yticks([])
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, 'message_drops.png'))
    plt.close()

def plot_throughput(tx_log, source , output_dir):
    """
    Plot V2X Hub and RSU throughput for each message type over time and saves to output_dir
    :param tx_log: Description
    :param rx_log: Description
    :param output_dir: Description
    """
    plt.set_loglevel('warning')
    plt.figure(figsize=(10, 6))     #
    # Convert Timestamp to datetime for better x-axis representation
    tx_log['DateTime'] = pd.to_datetime(tx_log['Timestamp'], unit='ms')
    markers = ['o', '^', 's', 'D', '*']
    marker_index = 0
    for message_id in tx_log['Message Type'].unique():
        subset_tx = tx_log[tx_log['Message Type'] == message_id]
        subset_tx_throughput = subset_tx.set_index('DateTime').resample('1s').size()
        plt.scatter(subset_tx_throughput.index, subset_tx_throughput.values, label= message_id + ' Throughput' , marker=markers[marker_index])
        marker_index = (marker_index + 1) % len(markers)
    plt.title( source + ' Throughput Over Time')
    plt.xlabel('Time')
    plt.ylabel('Messages per Second')
    # Include y marks for 0-12 
    plt.yticks(range(0, 12))
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(output_dir, f'{source.lower()}_throughput.png'))
    plt.close()
    
def main():
    """
    Reads files from input_directory, calculates messaging performance metrics
    and generates plots saved to ./plots directory.
    """
    
    # Check for command-line arguments (from orchestrator)
    parser = argparse.ArgumentParser(
        description='Analyze V2X messaging performance from log files.'
    )
    parser.add_argument('--input-file-one')
    parser.add_argument('--input-file-two')
    parser.add_argument("--output-dir", type=Path, default=Path('.'), help="Output directory")
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

    # Use the passed directory or default to local folders
    data_dir = args.output_dir / 'data'
    plots_dir = args.output_dir / 'plots'
    data_dir.mkdir(parents=True, exist_ok=True)
    plots_dir.mkdir(parents=True, exist_ok=True)

    source_file, destination_file = select_log_files(args.input_file_one,args.input_file_two)
    if source_file is None or destination_file is None:
        print('File selection cancelled.')
        return
    logging.debug('Reading source log file: %s', source_file)
    tx_logs = read_log_to_dataframe(source_file, data_dir)
    logging.debug('Reading destination log file: %s', destination_file)
    rx_logs = read_log_to_dataframe(destination_file, data_dir)
    # Debug log first 5 rows of each dataframe
    logging.debug('V2X Hub TX Logs:\n%s', tx_logs.head())
    logging.debug('RSU TX Logs:\n%s', rx_logs.head())
    message_drop_df, latency_df = calculate_messsage_performance(tx_logs, rx_logs)
    logging.debug('Message Latency for first 5 messages:\n%s', latency_df.head())
    logging.info(latency_df.describe(percentiles=[.95, .99]))
    logging.info(message_drop_df.describe())
    plot_latency(latency_df, plots_dir)
    plot_throughput(tx_logs, 'V2X Hub', plots_dir)
    plot_throughput(rx_logs, 'RSU', plots_dir)
    if not message_drop_df.empty:
        logging.info('Message Drop detected for %d messages.', len(message_drop_df))
        plot_message_drop(message_drop_df, plots_dir)

    # Formatted output for csv testing spreadsheet
    if not latency_df.empty:
        avg_latency = latency_df['Latency (ms)'].mean()
        # Extract source/destination names from the filenames
        src_name = os.path.basename(source_file).replace('decoded_', '').replace('.log', '')
        dst_name = os.path.basename(destination_file).replace('decoded_', '').replace('.log', '')
        
        #CSV Output Line
        print(f"\nRESULT_SUMMARY: {src_name} -> {dst_name}: {avg_latency} ms")

if __name__ == "__main__":
    main()

