import Vue from 'vue'
import App from './App.vue'
import router from './router'
import axios from 'axios'

import vuetify from './plugins/vuetify'
import RTSP2Web from 'rtsp2web'

Vue.prototype.$axios = axios

Vue.config.productionTip = false
Vue.config.performance = true

const port = 8033
new RTSP2web({
    port
})

Vue.use(vuetify)
new Vue({
    router,
    vuetify,
    render: (h) => h(App),
}).$mount('#app')
