<?php

class schema__alternate_name extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Synonyms';

  // The default description for this field.
  public static $description = 'An alternative name (synonym or alias) to this record.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'schema',
    // The name of the term.
    'term_name' => 'alternateName',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'alternateName',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  );

  // The default widget for this field.
  public static $default_widget = 'schema__alternate_name_widget';

  // The default formatter for this field.
  public static $default_formatter = 'schema__alternate_name_formatter';


  /**
   *
   * @see TripalField::load()
   */
  public function load($entity) {
    $record = $entity->chado_record;
    $base_table = $this->instance['settings']['base_table'];

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];

    // Get the PKey for this table
    $schema = chado_get_schema($field_table);
    $pkey = $schema['primary key'][0];

    // Get the FK that links to the base record.
    $schema = chado_get_schema($field_table);
    $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);
    $fkey_rcolumn = $schema['foreign keys'][$base_table]['columns'][$fkey_lcolumn];

    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0] = array(
      'value' => '',
      'chado-' . $field_table . '__' . $pkey => '',
      'chado-' . $field_table . '__' . $fkey_lcolumn => '',
      'chado-' . $field_table . '__' . 'synonym_id' => '',
      'chado-' . $field_table . '__' . 'pub_id' => '',
      'chado-' . $field_table . '__' . 'is_current' => '',
      'chado-' . $field_table . '__' . 'is_internal' => '',
      'name' => '',
      'type_id' => '',
      // Ignore the synonym_sgml column for now.
    );

    $linker_table = $base_table . '_synonym';
    $options = array('return_array' => 1);
    $record = chado_expand_var($record, 'table', $linker_table, $options);
    if (count($record->$linker_table) > 0) {
      $i = 0;
      foreach ($record->$linker_table as $index => $linker) {
        $synonym = $linker->synonym_id;
        $entity->{$field_name}['und'][$i] = array(
          'value' => $synonym->name,
          'chado-' . $field_table . '__' . $pkey => $linker->$pkey,
          'chado-' . $field_table . '__' . $fkey_lcolumn => $linker->$fkey_lcolumn->$fkey_lcolumn,
          'chado-' . $field_table . '__' . 'synonym_id' => $synonym->synonym_id,
          'chado-' . $field_table . '__' . 'pub_id' => $linker->pub_id->pub_id,
          'chado-' . $field_table . '__' . 'is_current' => $linker->is_current,
          'chado-' . $field_table . '__' . 'is_internal' => $linker->is_internal,
          'name' => $synonym->name,
          'type_id' => $synonym->type_id->cvterm_id,
        );
        $i++;
      }
    }
  }

  /**
   * @see ChadoField::query()
   */
  public function query($query, $condition) {
    $syn_linker = $this->instance['settings']['chado_table'];
    $base_table = $this->instance['settings']['base_table'];
    $bschema = chado_get_schema($base_table);
    $bpkey = $bschema['primary key'][0];
    $alias = 'syn_linker';
    $operator = $condition['operator'];

    if ($condition['column'] == 'alternatename') {
      $query->join($syn_linker, $alias, "base.$bpkey = $alias.$bpkey");
      $query->join('synonym', 'SYN', "SYN.synonym_id = $alias.synonym_id");
      $query->condition("SYN.name", $condition['value']);
    }
  }

}