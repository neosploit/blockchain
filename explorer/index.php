<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <title>Blockchain Explorer</title>
        <link href="https://fonts.googleapis.com/css?family=Roboto+Slab:400,700&display=swap" rel="stylesheet">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css">
        <link rel="stylesheet" href="css/style.css">
    </head>
    <body>
    <?php include 'includes/header.php'; ?>
    <?php

    // $json = file_get_contents('assets/json/blockchain.json');
    $curl = curl_init("http://127.0.0.1:55000/blockchain"); 
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    $json = curl_exec($curl);
    curl_close();

    $blockchain = json_decode($json);
    $number_of_blocks = count($blockchain);
    ?>
    <section class="blockchain-section">
        <div class="container-fluid">
            <div class="row">
                <div class="col-12 col-sm-6 col-md-5">
                    <h2>Latest Blocks</h2>
                    <table>
                        <thead>
                            <tr>
                                <th>ID</th>
                                <th>Nonce</th>
                                <th>Timestamp</th>
                                <th>Hash</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php
                            $transactions = [];
                            for ($i = ($number_of_blocks-1); $i>$number_of_blocks-11 && $i>=0; $i--) {
                                array_push($transactions, $blockchain[$i]->transactions);
                                $blockchain[$i]->timestamp = date('Y-m-d H:i:s', (int) $blockchain[$i]->timestamp);
                                ?>
                                <tr>
                                    <td><?php echo $blockchain[$i]->id; ?></td>
                                    <td><?php echo $blockchain[$i]->nonce; ?></td>
                                    <td><?php echo $blockchain[$i]->timestamp; ?></td>
                                    <td><a href="block.php?id=<?php echo $blockchain[$i]->id; ?>"><?php echo $blockchain[$i]->hash; ?></a></td>
                                </tr>
                                <?php 
                            } 
                            ?>
                        </tbody>
                    </table>
                </div>
                <div class="col-12 col-sm-6 col-md-7">
                    <h2>Latest Transactions</h2>
                    <?php 
                    $final_transactions = [];
                    $timestamps = [];
                    foreach ($transactions as $transaction) {
                        for ($i=0; $i<count($transaction); $i++) {
                            array_push($final_transactions, $transaction[$i]);
                            array_push($timestamps, $transaction[$i]->timestamp);
                        }
                    }
                    array_multisort($final_transactions, SORT_DESC, $timestamps);
                    ?>
                    <table>
                        <thead>
                            <tr>
                                <th>Sender</th>
                                <th>Receiver</th>
                                <th>Timestamp</th>
                                <th>Amount</th>
                            </tr>
                        </thead>
                        <tbody>
                            <?php
                            $transactions_counter = 1;
                            foreach ($final_transactions as $transaction) {
                                $transaction->timestamp = date('Y-m-d H:i:s', (int) $transaction->timestamp);
                                if ($transactions_counter > 10) break;
                                ?>
                                <tr>
                                    <td><?php echo $transaction->sender; ?></td>
                                    <td><?php echo $transaction->receiver; ?></td>
                                    <td><?php echo $transaction->timestamp; ?></td>
                                    <td><?php echo $transaction->amount; ?></td>
                                </tr>
                                <?php
                                $transactions_counter++;
                                }
                            ?>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </section>
    <?php include 'includes/footer.php'; ?>
    <?php include 'includes/common_scripts.php'; ?>
    </body>
</html>
