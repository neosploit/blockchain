<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <title>Blockchain Explorer Search</title>
        <link href="https://fonts.googleapis.com/css?family=Roboto+Slab:400,700&display=swap" rel="stylesheet">
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css">
        <link rel="stylesheet" href="css/style.css">
    </head>
    <body>
    <?php include 'includes/header.php'; ?>
    <?php

    //$json = file_get_contents('assets/json/blockchain.json');
    $curl = curl_init("http://127.0.0.1:55000/blockchain"); 
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    $json = curl_exec($curl);
    curl_close();

    $blockchain = json_decode($json);
    $search_text = $_POST['search'];
    $search_text_length = strlen($search_text);
    $type_of_search = (is_numeric($search_text) &&  $search_text_length<32) ? 1 : 2; // 1 by id, 2 by address
    $searched_blocks_by_id = [];
    $searched_blocks_by_address = [];
    foreach ($blockchain as $block) {
        if ($type_of_search == 1){
            if($block->id == (int) $search_text){
                array_push($searched_blocks_by_id, $block);
            }
        }
        else {
            foreach ($block->transactions as $transaction) {
                if ($transaction->sender == $search_text || $transaction->receiver == $search_text){
                    array_push($searched_blocks_by_address, $transaction);
                }
            }
        }
    }
    ?>
                            
    <section class="blockchain-section">
        <div class="container-fluid">
            <div class="row">
                <div class="col-12">
                    <h1 class="searched-text">Results for "<?php echo $search_text; ?>"</h1>
                    <?php if (!empty($searched_blocks_by_id)) { 
                        $searched_blocks_by_id[0]->timestamp = date('Y-m-d H:i:s', (int) $searched_blocks_by_id[0]->timestamp);
                    ?>
                        <h2>Block</h2>
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
                            <tr>
                                <td><?php echo $searched_blocks_by_id[0]->id; ?></td>
                                <td><?php echo $searched_blocks_by_id[0]->nonce; ?></td>
                                <td><?php echo $searched_blocks_by_id[0]->timestamp; ?></td>
                                <td><a href="block.php?id=<?php echo $searched_blocks_by_id[0]->id; ?>"><?php echo $searched_blocks_by_id[0]->hash; ?></td>
                            </tr>
                            </tbody>
                        </table>
                    <?php } ?>
                    <?php if (!empty($searched_blocks_by_address)) { ?>
                        <h2>Transactions</h2>
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
                            foreach ($searched_blocks_by_address as $transaction) {
                                $transaction->timestamp = date('Y-m-d H:i:s', (int) $transaction->timestamp);    
                            ?>
                                <tr>
                                    <td><?php echo $transaction->sender; ?></td>
                                    <td><?php echo $transaction->receiver; ?></td>
                                    <td><?php echo $transaction->timestamp; ?></td>
                                    <td><?php echo $transaction->amount; ?></td>
                                </tr>
                                <?php } ?>
                            </tbody>
                        </table>
                    <?php } ?>
                    <?php if (empty($searched_blocks_by_id) && empty($searched_blocks_by_address)) echo '<h4>No results found.</h4>'; ?>
                </div>
            </div>
        </div>
    </section>
    <?php include 'includes/footer.php'; ?>
    <?php include 'includes/common_scripts.php'; ?>
    </body>
</html>
