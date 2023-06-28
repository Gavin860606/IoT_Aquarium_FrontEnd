<template>
    <div class="home">
        <div class="automode">
            <v-switch v-if="!toggling" v-model="automode" hide-details inset :label="`Switch: ${automode}`"
                @change="test()">
            </v-switch>
            <v-fade-transition hide-on-leave>
                <v-card v-if="toggling" append-icon="$close" class="toggle_card" elevation="16" max-width="200" :color="'#FFFFFF'">
                    123
                </v-card>
            </v-fade-transition>
        </div>
        <div class="title">Last Update:
            <p id="intext">{{ this.last_update }}</p>
        </div>
        <div class="title">Temperature:
            <p id="intext">{{ this.temperature }}°C</p>
        </div>
        <div class="title">FAN pwm:
            <p id="intext">{{ this.pwm }}</p>
        </div>

        <div class="bbtn"><v-btn :disabled="btnDisable" @click="clk">Refresh</v-btn></div>

    </div>
</template>

<script>
// @ is an alias to /src

export default {
    name: 'Home',
    data() {
        return {
            model: true,
            temperature: 0,
            last_update: 0,
            pwm: 0,
            automode: null,
            btnDisable: false,
            toggling: false,
        }
    },
    watch: {
    },
    components: {},
    methods: {
        clk() {
            this.$axios
                .get(`https://wemosd1dp.azurewebsites.net/`)
                .then(result => {
                    console.log(result);
                    this.temperature = result.data.temperature.toFixed(2)
                    this.last_update = result.data.last_update
                    this.pwm = result.data.pwm
                    this.automode = result.data.automode
                })
                .catch(err => {
                    console.log(err);
                });
            // this.btnDisable = !this.btnDisable
        },

        test() {
            this.toggling = true;
            let mode = this.automode ? 1 : 0;
            console.log("mode:", mode);
            this.$axios
                .post(`https://localhost:80/api/changemode`, {
                    mode: mode,
                })
                .then(result => {
                    console.log(result);
                    this.temperature = result.data.temperature.toFixed(2)
                    this.last_update = result.data.last_update
                    this.pwm = result.data.pwm
                    this.automode = result.data.automode
                    setTimeout(() => {
                        this.toggling = false;
                    }, 500);
                })
                .catch(err => {
                    this.automode = !this.automode
                    setTimeout(() => {
                        this.toggling = false;
                    }, 500);
                    console.log(err);
                });
        }
    },
    created() {
        console.log('=== Home created! === ')
        this.$axios
            .get(`https://wemosd1dp.azurewebsites.net/`)
            .then(result => {
                this.temperature = result.data.temperature.toFixed(2)
                this.last_update = result.data.last_update
                this.pwm = result.data.pwm
                this.automode = result.data.automode
            })
            .catch(err => {
                console.log(err);
            });
    },
    beforeUpdate() {
        console.log('=== Home beforeUpdate! === ')
    },
    updated() {
        console.log('=== Home updated! === ')
    },
}
</script>
<style lang="scss" scoped>
* {
    // border: 1px solid #f00;
    color: #fff;
}

.home {
    width: 100vw;
    height: 100vh;
    position: relative;
    // background-color: white;
    background-image: linear-gradient(0deg, rgb(68, 137, 194) 0%, rgb(18, 99, 136) 100%);
    background-size: cover;
}

.title {
    margin-left: 50px;
    padding-top: 10px;
    color: rgb(255, 255, 255);
}

#intext {
    font-size: 20px;
}

.bbtn {
    border-radius: 5px;
    height: 25px;
    margin-left: 40px;
}

.toggle_card {
    color: black;
    background-color: beige;
    position: relative;
    height: 100px;
    margin-top: 50px;
}

.automode {

    margin-left: 100px;
}
</style>
