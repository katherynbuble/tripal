<?php

/**
 * Launchpad for feature administration.
 *
 * @ingroup tripal_feature
 */
function tripal_entities_admin_view() {

  $form = drupal_get_form('tripal_entities_admin_bundles_form');
  $output = drupal_render($form) . "<br>[ Image Place Holder for Data Type Summary ]<br>";

  // set the breadcrumb
  $breadcrumb = array();
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Biological Data', 'admin/tripal/bundles');
  drupal_set_breadcrumb($breadcrumb);

/*   // Add the view
  $view = views_embed_view('tripal_feature_admin_features','default');
  if (isset($view)) {
    $output .= $view;
  }
  else {
    $output .= '<p>The Feature module uses primarily views to provide an '
      . 'administrative interface. Currently one or more views needed for this '
        . 'administrative interface are disabled. <strong>Click each of the following links to '
          . 'enable the pertinent views</strong>:</p>';
    $output .= '<ul>';
    $output .= '<li>'.l('Features View', 'admin/tripal/chado/tripal_feature/views/features/enable').'</li>';
    $output .= '</ul>';
  }

  // Add a summary chart.
  //-----------------------------------
  $output .= theme('tripal_feature_bar_chart_type_organism_summary');
  drupal_add_js('
    Drupal.behaviors.tripalFeature_moveAdminSummaryChart = {
      attach: function (context, settings) {
        jQuery("#tripal-feature-admin-summary").insertBefore( jQuery(".view-filters") );
    }};
  ', 'inline'); */

  return $output;
}

/**
 *
 */
function tripal_entities_content_view() {
  $form = drupal_get_form('tripal_entities_content_overview_form');
  $output = drupal_render($form);

  // set the breadcrumb
  $breadcrumb = array();
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  drupal_set_breadcrumb($breadcrumb);

  return $output;

}

/**
 *
 * @param unknown $form
 * @param unknown $form_state
 * @return multitype:
 */
function tripal_entities_content_overview_form($form, &$form_state) {
  $form = array();

  $entities = db_select('tripal_entity', 'td')
    ->fields('td')
    ->orderBy('created', 'DESC')//ORDER BY created
    ->range(0,25)
    ->execute();

  $headers = array('Title', 'Vocabulary', 'Term', 'Author', 'Status', 'Updated', 'Operations');
  $rows = array();
  while ($entity = $entities->fetchObject()) {
    $author = user_load($entity->uid);
    $rows[] = array(
      l($entity->title, 'data/' . $entity->id),
      $entity->type,
      $entity->bundle,
      l($author->name, 'user/' . $entity->uid),
      $entity->status == 1 ? 'published' : 'unpublished',
      format_date($entity->changed, 'short'),
      l('edit', 'data/' . $entity->id . '/edit') . '&nbsp;&nbsp;' .
      l('delete', 'data/' . $entity->id . '/delete')
    );
  }

  $table_vars = array(
    'header'     => $headers,
    'rows'       => $rows,
    'attributes' => array(),
    'sticky'     => TRUE,
    'caption'    => '',
    'colgroups'  => array(),
    'empty'      => '',
  );
  $results_table = theme('table', $table_vars);

  $form['results'] = array(
    '#markup' => $results_table
  );
  return $form;
}

/**
 *
 * @param unknown $form
 * @param unknown $form_state
 * @return multitype:
 */
function tripal_entities_admin_bundles_form($form, &$form_state) {
  $form = array();

  // Set the defaults.
  $cv_id = NULL;
  $term_name = NULL;

  // Set defaults using the form state.
  if (array_key_exists('values', $form_state)) {
    $cv_id = array_key_exists('cv_id', $form_state['values']) ? $form_state['values']['cv_id'] : NULL;
    $term_name = array_key_exists('term_name', $form_state['values']) ? $form_state['values']['term_name'] : NULL;
  }

  // Let the user select the vocabulary and tripal_entity but only if they haven't
  // already selected a tripal_entity.
  $sql = "
    SELECT CV.cv_id, CV.name
    FROM {cv} CV
    ORDER BY CV.name
  ";
  $vocabs = chado_query($sql);
  $cvs = array();
  while ($vocab = $vocabs->fetchObject()) {
    $cvs[$vocab->cv_id] = $vocab->name;
  }
  $form['cv_id'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary'),
    '#options' => $cvs,
    '#required' => FALSE,
    '#description' => t('Select a vocabulary to view potential data types in the chart below. Limit the chart to only published data types by selecting the checkbox.'),
    '#default_value' => $cv_id,
    '#ajax' => array(
      'callback' => "tripal_entities_admin_bundles_form_ajax_callback",
      'wrapper' => 'tripal_entities_admin_bundles_form',
      'effect' => 'fade',
      'method' => 'replace'
    )
  );

  $form['refresh_bundles'] = array(
    '#type' => 'submit',
    '#value' => t('Refresh Data Types'),
    '#name' => 'refresh_bundles',
  );

  $form['publish_new_data'] = array(
    '#type' => 'submit',
    '#value' => t('Publish New Data'),
    '#name' => 'publish_new_data',
  );

  $form['#prefix'] = '<div id="tripal_entities_admin_bundle_form">';
  $form['#suffix'] = '</div>';
  return $form;
}

/**
 * Submit a job to populate the entity tables
 * This operation makes available data types in the database publishable
 */
function tripal_entities_admin_bundles_form_submit($form, $form_state) {
  global $user;
  if ($form_state['clicked_button']['#name'] == 'refresh_bundles') {
    tripal_add_job('Create publishable data types', 'tripal_entity', 'tripal_entities_populate_entity_tables', array(), $user->uid);
  }
  if ($form_state['clicked_button']['#name'] == 'publish_new_data') {
  }
}

/**
 *
 * @param unknown $form
 * @param unknown $form_state
 * @return multitype:
 */
function tripal_entities_admin_publish_form($form, &$form_state) {
  $form = array();

  // Set the defaults.
  $cv_id = NULL;
  $term_name = NULL;

  // Set defaults using the form state.
  if (array_key_exists('values', $form_state)) {
    $cv_id = array_key_exists('cv_id', $form_state['values']) ? $form_state['values']['cv_id'] : NULL;
    $term_name = array_key_exists('term_name', $form_state['values']) ? $form_state['values']['term_name'] : NULL;
  }

  // Let the user select the vocabulary and tripal_entity but only if they haven't
  // already selected a tripal_entity.
  $sql = "
    SELECT CV.cv_id, CV.name
    FROM {tripal_vocabulary} TET
      INNER JOIN {cv} CV on CV.cv_id = TET.cv_id
    ORDER BY CV.name
  ";
  $vocabs = chado_query($sql);
  $cvs = array();
  while ($vocab = $vocabs->fetchObject()) {
    $cvs[$vocab->cv_id] = $vocab->name;
  }
  $form['cv_id'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary'),
    '#options' => $cvs,
    '#required' => TRUE,
    '#description' => t('Select a vocabulary that contains the term you would like to set as publishable. Only vocabularies that are linked to data are shown.'),
    '#default_value' => $cv_id,
    '#ajax' => array(
      'callback' => "tripal_entities_admin_publish_form_ajax_callback",
      'wrapper' => 'tripal_entities_admin_publish_form',
      'effect' => 'fade',
      'method' => 'replace'
    )
  );

  // If we have a CV ID then we want to provide an autocomplete field
  if ($cv_id) {
    $form['cvterm_select']['term_name'] = array(
      '#title'       => t('Data Type'),
      '#type'        => 'textfield',
      '#description' => t("Please enter the name of the data type to set as publishable.  The data type must be a valid term in the selected vocabulary. This field will autopopulate as you type to help find available data types."),
      '#required'    => TRUE,
      '#default_value' => $term_name,
      '#autocomplete_path' => "admin/tripal/chado/tripal_cv/cvterm/auto_name/$cv_id",
    );

    $form['cvterm_select']['select_button'] = array(
      '#type' => 'submit',
      '#value' => t('Publish Data'),
      '#name' => 'publish',
    );
  }


  $form['#prefix'] = '<div id="tripal_entities_admin_publish_form">';
  $form['#suffix'] = '</div>';
  return $form;
}

/**
 * An Ajax callback for the tripal_entities_admin_publish_form..
 */
function tripal_entities_admin_publish_form_ajax_callback($form, $form_state) {
  // return the form so Drupal can update the content on the page
  return $form;
}
/**
 * Implements hook_validate() for the tripal_entities_admin_publish_form.
 *
 */
function tripal_entities_admin_publish_form_validate($form, &$form_state) {
  $cv_id = $form_state['values']['cv_id'];
  $term_name = $form_state['values']['term_name'];

  // Make sure the term_name is a real term in the vocabulary.
  $type = tripal_get_cvterm(array(
    'name' => $term_name,
    'cv_id' => $cv_id
  ));
  if (!$type) {
    form_set_error('term_name', t("The data type is not a valid name for the selected vocabulary."));
  }

  // Make sure the term is used in the site:
  $values = array(
    'cvterm_id' => $type->cvterm_id,
  );
  $bundles = chado_select_record('tripal_term', array('term_id'), $values);
  if (count($bundles) == 0) {
    form_set_error('term_name', t("The data type, %type, is not associated with data on this site and thus cannot be set as publishable.", array('%type' => $term_name)));
  }

  // Make sure the term is not already published.
  $values = array(
    'cvterm_id' => $type->cvterm_id,
    'publish' => 1,
  );
  $bundles = chado_select_record('tripal_term', array('term_id'), $values);
  if (count($bundles) > 0) {
    form_set_error('term_name', t("This data type is already set as publishable."));
  }
}
/**
 * Implements hook_submit() for the tripal_entities_admin_publish_form.
 *
 */
function tripal_entities_admin_publish_form_submit($form, &$form_state) {
  $cv_id = $form_state['values']['cv_id'];
  $term_name = $form_state['values']['term_name'];

  // Get the data type using the $term_name and $cv_id.
  $cvterm = chado_generate_var('cvterm', array('cv_id' => $cv_id, 'name' => $term_name));

  // Start the transaction.
  $transaction = db_transaction();
  try {

    // We don't need to check if the vocabulary is used because the
    // form only shows those vocabs that are used.

    // Mark this entity as published.
    $match = array('cv_id' => $cv_id);
    $values = array('publish' => 1);
    $success = chado_update_record('tripal_vocabulary', $match, $values);
    if (!$success) {
      throw new Exception('Cannot set the vocabulary as publishable');
    }

    // We have already checked in the validate if the term already exists
    // as a bundle. So, if we're here we can enable it.
    $match = array('cvterm_id' => $cvterm->cvterm_id);
    $values = array('publish' => 1);
    $success = chado_update_record('tripal_term', $match, $values);
    if (!$success) {
      throw new Exception('Cannot set the data type as publishable');
    }

    // Create the bundle name and entity type name.
    $bundle_name = $cvterm->dbxref_id->db_id->name . '_' . $cvterm->dbxref_id->accession;
    $entity_type_name = $cvterm->dbxref_id->db_id->name;

    // Clear the entity cache so that Drupal will read our
    // hook_entity_info() implementation which now will have the entities
    // described because we set the publish column to 1 in the tripal_term
    // table.
    global $language;
    $langcode = $language->language;
    cache_clear_all("entity_info:$langcode", 'cache');

    // The TripalBundle Entity manages the bundles we have available.
    // Therefore, we need to add a new entity for each bundle "type".
    $vals = array(
      'label' => $bundle_name  . ' (' . $cvterm->name . ')',
      'type' => $entity_type_name,
      'bundle' => $bundle_name,
      'data' => serialize(array()),
      'module' => 'tripal_entities'
    );
    $tripal_bundle = new TripalBundle($vals, $entity_type_name . '_bundle');
    $tripal_bundle->save();

    // Allow modules to now add fields to the bundle
    module_invoke_all('add_bundle_fields', $entity_type_name, $bundle_name, $cvterm);

    drupal_set_message(t('Data type, %type, is now set as publishable.', array('%type' => $term_name)));
  }
  catch (Exception $e) {
    $transaction->rollback();
    drupal_set_message('Failure publishing this data type: ' . $e->getMessage(), 'error');
    watchdog_exception('trp_entities', $e);
  }
}
/**
 *
 * @param $table
 * @param $entity_type
 * @param $bundle_name
 */
function tripal_entities_add_bundle_fields($entity_type_name, $bundle_name, $cvterm) {

  // Get the list of tables where this cvterm is used.
  $match = array('cvterm_id' => $cvterm->cvterm_id);
  $term = chado_select_record('tripal_term', array('*'), $match);
  $values = array('term_id' => $term[0]->term_id);
  $tables = chado_select_record('tripal_term_usage', array('*'), $values);

  // Iterate through the tables.
  foreach ($tables as $table) {
    $tablename = $table->data_table;
    $type_table = $table->type_table;
    $type_field = $table->field;

    // We only want to look at base tables.
    if ($tablename == 'cvterm_dbxref' || $tablename == 'cvterm_relationship' ||
        $tablename == 'cvtermpath' || $tablename == 'cvtermprop' || $tablename == 'chadoprop' ||
        $tablename == 'cvtermsynonym' || preg_match('/_relationship$/', $tablename) ||
        preg_match('/_cvterm$/', $tablename)) {
      continue;
    }

    // Iterate through the columns of the table and see if fields have been
    // created for each one. If not, then create them.
    $schema = chado_get_schema($tablename);
    $columns = $schema['fields'];
    foreach ($columns as $column_name => $details) {
      $field_name = $tablename . '__' . $column_name;
      $field = field_info_field($field_name);

      // Skip the primary key field.
      if ($column_name == $schema['primary key'][0]) {
        continue;
      }

      // Skip the type field.
      if ($tablename == $type_table and $column_name == $type_field) {
        continue;
      }

      // Determine if the field is required.
      $is_required = 0;
      if (array_key_exists('not null', $details) and $details['not null'] === TRUE) {
        $is_required = array_key_exists('default', $details) ? 0 : 1;
      }

      // Determine what type of field this should be.
      // Drupal data types are: https://www.drupal.org/node/159605.
      // Field types are here:  https://www.drupal.org/node/1879542

      // Create an array with information about this field.
      $field_info = array(
        'field_type' => '',
        'widget_type' => '',
        'settings' => array(),
        'description' => '',
        'label' => ucwords(preg_replace('/_/', ' ', $column_name)),
        'chado_table' => $tablename,
        'chado_column' => $column_name
      );

      // Alter the field info array dependiing on the column details.
      switch($details['type']) {
        case 'char':
          $field_info['field_type'] = 'text';
          $field_info['widget_type'] = 'text_textfield';
          $field_info['settings']['max_length'] = $details['length'];
          break;
        case 'varchar':
          $field_info['field_type'] = 'text';
          $field_info['widget_type'] = 'text_textfield';
          $field_info['settings']['max_length'] = $details['length'];
          break;
        case 'text':
          $field_info['field_type'] = 'text';
          $field_info['widget_type'] = 'text_textarea';
          break;
        case 'blob':
          // not sure how to support a blob field.
          continue;
          break;
        case 'int':
          $field_info['field_type'] = 'number_integer';
          $field_info['widget_type'] = 'number';
          break;
        case 'float':
          $field_info['field_type'] = 'number_float';
          $field_info['widget_type'] = 'number';
          $field_info['settings']['precision'] = 10;
          $field_info['settings']['scale'] = 2;
          $field_info['settings']['decimal_separator'] = '.';
          break;
        case 'numeric':
          $field_info['field_type'] = 'number_decimal';
          $field_info['widget_type'] = 'number';
          break;
        case 'serial':
          // Serial fields are most likely not needed as a field.
          break;
        case 'boolean':
          $field_info['field_type'] = 'list_boolean';
          $field_info['widget_type'] = 'options_onoff';
          $field_info['settings']['allowed_values'] = array(0 => "No", 1 => "Yes");
          break;
        case 'datetime':
          // Use the Drupal Date and Date API to create the field/widget
          $field_info['field_type'] = 'datetime';
          $field_info['widget_type'] = 'date_select';
          $field_info['settings']['increment '] = 1;
          $field_info['settings']['tz_handling'] = 'none';
          
          // TODO: Add settings so that the minutes increment by 1.
          // And turn off the timezone, as the Chado field doesn't support it.
          break;
      }

      // If we don't have a field type then we don't need to create a field.
      if (!$field_info['field_type']) {
        // If we don't have a field type but it is required and doesn't have
        // a default value then we are in trouble.
        if ($is_required and !array_key_exists('default', $details)) {
          throw new Exception(t('The %table.%field type, %type, is not yet supported for Entity fields, but it is required,',
            array('%table' => $tablename, '%field' => $column_name, '%type' => $details['type'])));
        }
        continue;
      }

      // If this field is a foreign key field then we will have a special custom
      // field provided by Tripal.
      $is_fk = FALSE;
      if (array_key_exists('foreign keys', $schema)) {
        foreach ($schema['foreign keys'] as $remote_table => $fk_details) {
          if (array_key_exists($column_name, $fk_details['columns'])) {
            $is_fk = TRUE;
          }
        }
      }

      // Allow other modules to alter the field information array.
      drupal_alter('chado_field', $field_info);

      // If the field doesn't exist then create it.
      if (!$field) {
        $field = array(
          'field_name' => $field_name,
          'type' => $field_info['field_type'],
          'cardinality' => 1,
          'locked' => FALSE,
          'storage' => array(
            'type' => 'field_chado_storage'
          ),
          'settings' => $field_info['settings'],
        );
        field_create_field($field);
      }

      // Attach the field to the bundle.
      $field_instance = array(
        'field_name' => $field_name,
        'label' => $field_info['label'],
        'description' => $field_info['description'],
        'widget' => array(
          'type' => $field_info['widget_type'],
          'settings' => array('display_label' => 1)
        ),
        'entity_type' => $entity_type_name,
        'required' => $is_required,
        'settings' => $field_info['settings'],
        'bundle' => $bundle_name,
      );
      field_create_instance($field_instance);
    }
  }
}

/**
 * Implements hook_chado_field_alter.
 *
 * This function is used when new Chado fields are addd to an Entity.  It
 * allows modules to customize the field, widget types and settings for
 * a field before it is created.
 *
 * @param $field
 */
function hook_chado_field_alter(&$field) {
  // TODO: add example code for how to use this hook.
}
/**
 *
 * @param unknown $form
 * @param unknown $form_state
 * @return multitype:
 */
function tripal_entities_admin_access_form($form, &$form_state) {
  $form = array();

  return $form;
}