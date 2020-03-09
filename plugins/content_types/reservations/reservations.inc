<?php
/**
 * @file
 * User reservation list.
 */

$plugin = array(
  'title' => t('User reservation list'),
  'description' => t('The current reservations for a user'),
  'single' => TRUE,
  'content_types' => array('list'),
  'required context' => new ctools_context_required(t('User'), 'user'),
  'category' => t('User'),
);

/**
 * Render the block.
 * @throws DingProviderUserException
 */
function ding_reservation_reservations_content_type_render($subtype, $conf, $panel_args, $context) {
  static $preload_reservations = TRUE;
  $account = isset($context->data) ? $context->data : NULL;
  $preload_ids = array();
  $reservation_list = array();

  $block = new stdClass();
  $block->module = 'ding_reservation';
  $block->delta = 'reservations';

  $list = ding_provider_invoke_page('reservation', 'list', $account);
  $items = array();
  switch ($conf['reservation_type_list']) {
    case DING_RESERVATION_READY:
      $block->title = t('Reservations ready for pickup');
      $block->content = t('There are no reservations ready for pickup.');
      $form_id = 'ding_reservation_reservations_ready_form';
      $items = $list[DING_RESERVATION_READY];
      break;

    default:
    case DING_RESERVATION_NOT_READY:
      $block->title = t('Reservations');
      $block->content = t('There are no reservations.');
      $form_id = 'ding_reservation_reservations_notready_form';
      $items = $list[DING_RESERVATION_NOT_READY];
      break;

    case DING_RESERVATION_INTERLIBRARY_LOANS:
      $block->title = t('Interlibrary loans');
      $block->content = t('There are no interlibrary loans.');
      $form_id = 'ding_reservation_reservations_ill';
      $items = $list[DING_RESERVATION_INTERLIBRARY_LOANS];
  }

  foreach ($items as $item) {
    if (!empty($item->__get('ding_entity_id'))) {
      $preload_ids[] = $item->__get('ding_entity_id');

      if (isset($conf['reservation_type_list'])) {
        $status = $item->__get(DING_RESERVATION_READY);
        $ready_for_pickup = $conf['reservation_type_list'] == DING_RESERVATION_READY && $status;
        $not_ready_for_pickup = !$status;

        $ill_status = $item->__get('ill_status');
        $interlibrary_loan = $conf['reservation_type_list'] == DING_RESERVATION_INTERLIBRARY_LOANS && !empty($ill_status);

        if ($ready_for_pickup || $not_ready_for_pickup || $interlibrary_loan) {
          $reservation_list[] = $item;
        }
      }
    }
    elseif (!empty($item->__get('title'))) {
      $reservation_list[] = $item;
    }
  }
  // Preload all ting objects.
  if ($preload_reservations) {
    ding_entity_load_multiple($preload_ids);
    $preload_reservations = FALSE;
  }
  if (count($reservation_list) > 0) {
    $block->content = ding_provider_get_form($form_id, $reservation_list, $conf['reservation_type_list']);
  }

  return $block;
}

/**
 * Adding the admin form, to be able to control the required context.
 */
function ding_reservation_reservations_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['reservation_type_list'] = array(
    '#type' => 'radios',
    '#title' => t('Reservation list type'),
    '#options' => array(
      'not_ready_for_pickup' => t('Not ready for pickup'),
      'ready_for_pickup' => t('Ready for pickup'),
      'interlibrary_loans' => t('Interlibrary loans'),
    ),
    '#default_value' => !empty($conf['reservation_type_list']) ? $conf['reservation_type_list'] : NULL,
  );
  return $form;
}

/**
 * Submit handler for the admin form.
 */
function ding_reservation_reservations_content_type_edit_form_submit(&$form, &$form_state) {
  foreach (element_children($form) as $key) {
    if (!empty($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
}
