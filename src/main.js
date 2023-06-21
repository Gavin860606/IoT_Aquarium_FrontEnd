import Vue from 'vue'
import App from './App.vue'
import router from './router'
import axios from 'axios'

import vuetify from './plugins/vuetify'

Vue.prototype.$axios = axios

Vue.config.productionTip = false
Vue.config.performance = true

Vue.use(vuetify)
new Vue({
    router,
    vuetify,
    render: (h) => h(App),
}).$mount('#app')
