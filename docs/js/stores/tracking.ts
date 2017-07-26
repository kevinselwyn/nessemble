import * as Reflux from 'reflux';

import TrackingActions from '../actions/tracking';

var TrackingStore = Reflux.createStore({
    listenables: [TrackingActions],

    init() {
        this.ga = (<any>window).ga;
        this.category = '';
        this.action = '';
        this.label = '';
        this.fields = {};

        if (!window.hasOwnProperty('ga')) {
            this.ga = () => {};
        }
    },

    onTrackEvent(category: string, action: string, label: string, fields: object) {
        this.category = category;
        this.action = action;
        this.label = label;
        this.fields = fields;

        this.ga('send', {
            hitType: 'event',
            eventCategory: category,
            eventAction: action,
            eventLabel: label,
            fieldsObject: fields
        });

        this._trigger();
    },

    _trigger() {
        this.trigger({
            category: this.category,
            action: this.action,
            label: this.label,
            fields: this.fields
        });
    }
});

export default TrackingStore;
