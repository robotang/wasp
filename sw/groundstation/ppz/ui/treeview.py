import gobject
import gtk

class MessageTreeStore(gtk.TreeStore):

    NAME_IDX,       \
    OBJECT_IDX,     \
    EDITABLE_IDX,   \
    VALUE_IDX,      \
    FIELDS_IDX =    range(5)

    def __init__(self):
        gtk.TreeStore.__init__(self, 
            str,        #NAME, message.name or field.name
            object,     #OBJECT, message or field underlying python object
            bool,       #EDITABLE, true for fields
            object,     #VALUE, value of field
            object)     #FIELDS, list of fields, only set for message rows

        self._message_ids = {}

    def add_message(self, message):
        fields = message.get_fields()
        m = self.append(None, (message.name, message, False, None, fields))
        for f in fields:
            self.append(m, (f.name, f, True, f.get_default_value(), None))
        return m

    def update_message(self, message, payload, add=True):
        if message.id not in self._message_ids:
            if add:
                self._message_ids[message.id] = self.add_message(message)
            else:
                return

        #get the tree row that represents this message
        _iter = self._message_ids[message.id]
        #and the number of children rows, i.e. fields, of the message
        nkids = self.iter_n_children(_iter)

        vals = message.unpack_values(payload)
        assert len(vals) == nkids

        for i in range(nkids):
            self.set_value(
                   self.iter_nth_child(_iter, i),
                   MessageTreeStore.VALUE_IDX,
                   vals[i])

class MessageTreeView(gtk.TreeView):
    def __init__(self, messagetreemodel, editable=True):
        gtk.TreeView.__init__(self, messagetreemodel)

        self.insert_column_with_attributes(-1, "Name",
                gtk.CellRendererText(),
                text=MessageTreeStore.NAME_IDX)

        rend = gtk.CellRendererText()
        rend.connect("edited", self._value_edited_cb, messagetreemodel)
        if editable:
            col = gtk.TreeViewColumn("Value", rend, editable=MessageTreeStore.EDITABLE_IDX)
        else:
            col = gtk.TreeViewColumn("Value", rend)
        col.set_cell_data_func(rend, self._get_field_value)
        self.append_column(col)

        self.get_selection().set_mode(gtk.SELECTION_SINGLE)

    def _value_edited_cb(self, cellrenderertext, path, new_text, model):
        _iter = model.get_iter(path)
        field = model.get_value(_iter, MessageTreeStore.OBJECT_IDX)
        old = model.get_value(_iter, MessageTreeStore.VALUE_IDX)

        #user deleted value, reset to default
        if new_text == "":
            value = field.get_default_value()
        else:
            #update value if valid, otherwise keep current value
            value = field.interpret_value_from_user_string(new_text, default=old)

        model.set_value(_iter, MessageTreeStore.VALUE_IDX, value)

    def _get_field_value(self, column, cell, model, _iter):
        value = model.get_value(_iter, MessageTreeStore.VALUE_IDX)

        txt = ""
        #make sure we dont set the value on top level, aka message, rows
        if model.iter_depth(_iter) == 1:
            field = model.get_value(_iter, MessageTreeStore.OBJECT_IDX)
            if value == None:
                value = field.get_default_value()
            txt = field.get_printable_value(value)
        cell.set_property("text", txt)

    def get_selected_message_and_values(self):
        model, _iter = self.get_selection().get_selected()

        #make sure something selected
        if not _iter:
            return None, None

        #make sure a message, not a field is selected
        if model.iter_depth(_iter) == 1:
            _iter = model.iter_parent(_iter)

        message = model.get_value(_iter, MessageTreeStore.OBJECT_IDX)
        fields = model.get_value(_iter, MessageTreeStore.FIELDS_IDX)
        values = []

        _iter = model.iter_children(_iter)
        while _iter:
            val = model.get_value(_iter, MessageTreeStore.VALUE_IDX)
            values.append(val)
            _iter = model.iter_next(_iter)

        return message, values

